#include "ball.h"

// returns only positive
int getRandomDir(int max) {
    return (rand() % max) + 1;
}

// returns positive or negative
int getRandomSignedDir(int max) {
    int rDir = getRandomDir(max);
    return (rand() % 5 == 2) ? rDir : -rDir;
}

Ball* ballInit() {
    srand(time(NULL));

    Ball* ball = malloc(sizeof(Ball));
    if (!ball) {
        perror("malloc");
        exit(1);
    }

    ball->x = ball->originX = WIDTH/2;
    ball->y = ball->originY = HEIGHT/2;

    ball->velx = getRandomDir(MAX_VEL_X);
    ball->vely = getRandomSignedDir(MAX_VEL_Y);

    ball->tick_rate = TICKS_PER_SEC;

    return ball;
}

static void SignalPass(Ball* ball, int socket_fd) {
    char passMsg[256];
    snprintf(passMsg, sizeof(passMsg), "PASS %d %d %d %d %d\n",
        ball->x,
        ball->y,
        ball->velx,
        ball->vely,
        ball->tick_rate
    );
    write(socket_fd, passMsg, strlen(passMsg));
}

void HideBall(Ball* ball) {
    ball->x = WIDTH/2;
    ball->y = HEIGHT/2;

    ball->velx = ball->vely = 0;
}

void checkCollision(Ball* ball, int playerId, int socket_fd) {
    if (!ball) return;
    
    // top and bottom wall hits
    if (ball->y <= 1 || ball->y >= HEIGHT-2) {
        ball->vely *= -1;
    }

    if (playerId == 1) {
        // player 2
        if (ball->x <= 1) {
            // cross court threshold
            SignalPass(ball, socket_fd);
            HideBall(ball);
        }
    } else {
        // player 1
        if (ball->x >= WIDTH) {
            // cross court threshold
            SignalPass(ball, socket_fd);
            HideBall(ball);
        }
    }
}

void hitPaddle(Ball* ball) {
    ball->velx *= -1;

    // make the ball move faster on screen
    ball->tick_rate = TICKS_PER_SEC + (rand() % 11);
    setTicker(1000/ball->tick_rate);
}
