#ifndef PONG_BALL
#define PONG_BALL

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>

#include "rules.h"

typedef struct {
    int x;
    int y;

    int originX;
    int originY;

    int velx;
    int vely;
} Ball;

Ball* ballInit();
void reset(Ball* ball);
void checkCollision(Ball* ball);
void hitPaddle(Ball* ball);

#endif
