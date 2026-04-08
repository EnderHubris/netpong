#ifndef PONG_BALL
#define PONG_BALL

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
void HideBall(Ball* ball);
void reset(Ball* ball);
void checkCollision(Ball* ball, int playerId, int socket_fd);
void hitPaddle(Ball* ball);

#endif
