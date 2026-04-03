#include "game.h"

// initialize pongState from game.h definition
Pong pongState = {
    NULL,
    NULL,
    NULL,
    0,
    -1
};

static void refreshCourt() {
    refresh();
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

    wrefresh(pongState.scene);
}

/**
 * Prepare the curses window and game scene
 * with the 6-row long paddle "#" and "O" ball at
 * the center of the screen
 */
void setup(int playerId) {
    // initialize game state components
    pongState.paddle = paddleInit();

    // set playerId
    pongState.playerId = playerId;

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
    
    refreshCourt();

    // set up the tick system
    signal(SIGALRM, ballHandle);
}

/**
 * ball is served at a random speed in a random
 * direction
 */
void serve() {
    // put the ball at mid-point of game
    // and serve with random velocity
    if (!pongState.ball) {
        // create new ball
        pongState.ball = ballInit(WIDTH,HEIGHT);
    }
    setTicker(1000/TICKS_PER_SEC);
}

/**
 * clean-up curses
 */
void cleanCurses() {
    if (pongState.scene) {
        delwin(pongState.scene);
    }
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
    return pongState.ball->x < pongState.paddle->x;
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
        ball->x += (ball->velx > 0) ? 1 : -1;
        mvwaddch(pongState.scene,ball->y,ball->x,'O');
        
        if (paddleContact(pongState.paddle, ball->x, ball->y)) {
            hitPaddle(ball);
        } else {
            checkCollision(ball);
        }
    }

    for (int i = 0; i < abs(ball->vely); ++i) {
        mvwaddch(pongState.scene,ball->y,ball->x,' ');
        ball->y += (ball->vely > 0) ? 1 : -1;
        mvwaddch(pongState.scene,ball->y,ball->x,'O');
    }

    if (!ballInPlay(ball)) {
        ++pongState.ballCount;
        reset(ball);
    }

    refreshCourt();
    signal(SIGALRM, ballHandle);
}

void paddleHandle() {
    drawPaddle(pongState.paddle);
}
