#ifndef PONG_PADDLE
#define PONG_PADDLE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "rules.h"

#define PADDLE_SIZE 6
#define PADDLE_BASE_X 8

/**
 * Paddle has a pivot (x,y) where x is fixed
 * and the user input changes the y value
 */
typedef struct {
    int x;
    int y;
} Paddle;

Paddle* paddleInit();
void paddleUp(Paddle* paddle);
void paddleDown(Paddle* paddle);
int paddleContact(Paddle* paddle, int playerId, int ballX, int ballY);

#endif
