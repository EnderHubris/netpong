#include "game.h"
#include "../server/client.h"

// initialize pongState from game.h definition
Pong pongState = { 0 };
char* scoreText = NULL;

static void SendMissSignal(int socket_fd) {
    char scoreMsg[124];
    snprintf(scoreMsg, sizeof(scoreMsg), "SCORE %d\n",
        (pongState.playerId + 1) % 2
    );
    write(socket_fd, scoreMsg, strlen(scoreMsg));
}

static void beginGame() {
    pongState.running = 1;
}

static void refreshCourt() {
    refresh();

    // clear the window scene
    werase(pongState.scene);
    // redraw the game boundary
    box(pongState.scene, 0, 0);

    // remove one of the columns
    // based on playerId
    char dividerChar = '.';
    for (int i = 1; i < HEIGHT-1; ++i) {
        if (pongState.playerId == 0) {
            mvwaddch(pongState.scene, i, WIDTH-1, dividerChar);
        }
        if (pongState.playerId == 1) {
            mvwaddch(pongState.scene, i, 0, dividerChar);
        }
    }

    // draw the ball on screen if it has a valid velocity
    if (pongState.ball && pongState.ball->velx != 0 && pongState.ball->vely != 0) {
        mvwaddch(pongState.scene, pongState.ball->y, pongState.ball->x, 'O');
    }

    // display score on screen
    /*
    werase(pongState.scoreWind);
    mvwprintw(pongState.scoreWind, 0, WIDTH/2, "%s", scoreText ? scoreText : "0:0");
    wrefresh(pongState.scoreWind);
    */

    // refresh window contents
    wrefresh(pongState.scene);
}

void checkForChange() {
    fd_set readfds;
    FD_ZERO(&readfds);                          // clear the set
    FD_SET(pongState.socket_fd, &readfds);      // add your socket

    struct timeval tv = {0,0};

    // select allows the program to monitor the socket_fd to check
    // whether of not the socket is ready for reading
    int r = select(pongState.socket_fd+1, &readfds, NULL, NULL, &tv);

    char buf[128];

    if (r > 0 && FD_ISSET(pongState.socket_fd, &readfds)) {
        ssize_t bytesRecv = read(pongState.socket_fd, buf, sizeof(buf)-1);

        if (bytesRecv > 0) {
            buf[bytesRecv] = 0;

            Strings res = SplitStr(buf, ' ');

            // the split results should have the format
            // [ TYPE DATA ]
            if (res.stringCount == 0) return;

            if (res.stringCount == 5 && strcmp(res.strs[0], "BALL") == 0) {
                int ballData[4] = {0};
                for (int i = 0; i < 4; ++i) {
                    ballData[i] = atoi(res.strs[i+1]);
                }

                if (!pongState.ball) return;
                
                // set the ball state from what is recv from the socket
                pongState.ball->x = ballData[0];
                pongState.ball->y = ballData[1];
                pongState.ball->velx = ballData[2];
                pongState.ball->vely = ballData[3];
            } else if (res.stringCount == 2 && strcmp(res.strs[0], "SCORE") == 0) {
                if (scoreText) {
                    free(scoreText); // free dynamic memory if needed
                    scoreText = NULL; // prevent use-after-free
                }

                int sLen = strlen(res.strs[1]) + 1; // room for null-terminating
                scoreText = malloc(sLen);
                strncpy(scoreText, res.strs[1], sLen-1); // cp string into buffer
                scoreText[sLen] = '\0';
            } else if (strcmp(res.strs[0], "SERVE") == 0) {
                reset(pongState.ball);
                ++pongState.ballCount;
            }
        } else if (bytesRecv == 0) {
            // server closed the socket
            pongState.running = 0;
        } else {
            // some error
            perror("read");
            pongState.running = 0;
        }
    }
}

/**
 * Prepare the curses window and game scene
 * with the 6-row long paddle "#" and "O" ball at
 * the center of the screen
 */
void setup(int playerId, char* hostStr, int port) {
    // initialize game state components
    pongState.paddle = paddleInit(playerId);

    // set playerId
    pongState.playerId = playerId;

    // player 1 socket
    pongState.socket_fd = RunClient(hostStr, port);
    if (pongState.socket_fd == -1) exit(1);

    // wait for player 2 to connect to start the pong game-loop
    if (playerId == 0) {
        signal(SIGUSR1, beginGame);
        while (!pongState.running) {
            pause();
        }
    }

    // initialize curses mode
    initscr();              // Start curses mode
    cbreak();               // Disable line buffering
    noecho();               // Don't echo user input
    start_color();          // Allow color pairs
    use_default_colors();   // Give access to default colors
    curs_set(0);            // Hide the cursor
    refresh();              // refresh the screen
    nodelay(stdscr, TRUE);  // make getch non-blocking
    
    // draw the game area box (h,w,y,x)
    pongState.scene = newwin(HEIGHT,WIDTH,0,0);
    pongState.scoreWind = newwin(2, WIDTH, HEIGHT, 0);
    
    refreshCourt();

    // set up the tick system
    signal(SIGALRM, ballHandle);
}

/**
 * ball is served at a random speed in a random
 * direction
 */
void serve(int playerId) {
    // put the ball at mid-point of game
    // and serve with random velocity
    if (!pongState.ball) {
        // create new ball
        pongState.ball = ballInit(WIDTH,HEIGHT);

        if (playerId == 1) {
            // hides the ball for player 2 visually
            pongState.ball->velx = pongState.ball->vely = 0;
        }
    }
    setTicker(1000/TICKS_PER_SEC);
}

/**
 * pong state clean-up
 */
void closeGame() {
    // close socket connection
    close(pongState.socket_fd);
    pongState.running = 0;
}

/**
 * clean-up curses
 */
void cleanCurses() {
    pongState.running = 0;
    
    if (pongState.scene) delwin(pongState.scene);
    if (pongState.scoreWind) delwin(pongState.scoreWind);
    pongState.scene = pongState.scoreWind = NULL;

    free(pongState.ball);
    free(pongState.paddle);
    pongState.ball = NULL;
    pongState.paddle = NULL;

    endwin(); // Stop curses mode
}

/**
 *
 */
int gameRunning(int* ch) {
    // grab player input
    *ch = getch(); // curses version of getchar
    return (pongState.ballCount < GAMEOVER) && ( *ch != (int)'q' );
}
int ballInPlay() {
    int inPlay = (pongState.playerId == 1) ? (
        // player 2
        pongState.ball->x < pongState.paddle->x
    ) : (
        // player 1
        pongState.ball->x > pongState.paddle->x
    );

    return inPlay;
}

void moveUp() {
    paddleUp(pongState.paddle);
}
void moveDown() {
    paddleDown(pongState.paddle);
}

/**
 * Draw Paddle based on its state
 */
static void drawPaddle(Paddle* paddle) {
    if (!paddle) return;
    if (!pongState.scene) return;

    // clear column where paddle is in
    for (int i = 1; i < HEIGHT-1; ++i) {
        mvwaddch(pongState.scene,i,paddle->x,' ');
    }

    for (int i = 0; i < PADDLE_SIZE; ++i) {
        int paddley = paddle->y + i;
        mvwaddch(pongState.scene,paddley,paddle->x,'#');
    }

    wrefresh(pongState.scene);
}

void ballHandle() {
    Ball* ball = pongState.ball;
    if (!ball) return;

    // disable the ballHandle execution from alarm signal
    signal(SIGALRM,SIG_IGN);

    for (int i = 0; i < abs(ball->velx); ++i) {
        mvwaddch(pongState.scene,ball->y,ball->x,' ');

        // update the ball x position
        if (ball->velx != 0 && ball->vely != 0) {
            ball->x += (ball->velx > 0) ? 1 : -1;
            mvwaddch(pongState.scene,ball->y,ball->x,'O');
        }
        
        if (paddleContact(pongState.paddle, pongState.playerId, ball->x, ball->y)) {
            hitPaddle(ball);
        } else {
            checkCollision(ball, pongState.playerId, pongState.socket_fd);
        }
    }

    // update the ball y position
    for (int i = 0; i < abs(ball->vely); ++i) {
        mvwaddch(pongState.scene,ball->y,ball->x,' ');

        if (ball->velx != 0 && ball->vely != 0) {
            ball->y += (ball->vely > 0) ? 1 : -1;
            mvwaddch(pongState.scene,ball->y,ball->x,'O');
        }
    }

    if (!ballInPlay(ball)) {
        ++pongState.ballCount;
        SendMissSignal(pongState.socket_fd);
        HideBall(pongState.ball);
    }

    refreshCourt();
    signal(SIGALRM, ballHandle);
}

void paddleHandle() {
    drawPaddle(pongState.paddle);
}