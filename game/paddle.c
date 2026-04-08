#include "paddle.h"

Paddle* paddleInit(int playerId) {
    Paddle* paddle = malloc(sizeof(Paddle));
    if (!paddle) {
        perror("malloc");
        exit(1);
    }
    
    // player 1 the paddle is on the left
    // player 2 the paddle is on the right
    paddle->x = (playerId == 1) ? WIDTH - (PADDLE_BASE_X+1) : PADDLE_BASE_X;
    paddle->y = 1;

    return paddle;
}

void paddleUp(Paddle* paddle) {
    if (!paddle) return;
    if (paddle->y > 1) {
        --paddle->y;
    }
}
void paddleDown(Paddle* paddle) {
    if (!paddle) return;
    if (paddle->y < HEIGHT - PADDLE_SIZE - 1) {
        ++paddle->y;
    }
}

/**
 * returns 1 if the contact is valid, otherwise 0
 */
int paddleContact(Paddle* paddle, int playerId, int ballX, int ballY) {
    if (!paddle) return 0;

    // ensure ball is within the column in-front of the paddle
    int containedX = (playerId == 1) ? (ballX == paddle->x-1) : (ballX == paddle->x+1);

    // ensure the ballY is contained within the paddle length
    int containedY = (
        ballY >= paddle->y
    ) && (
        ballY <= paddle->y + PADDLE_SIZE
    );

    return containedX && containedY;
}
