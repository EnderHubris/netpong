#ifndef PONG_GAME
#define PONG_GAME

#include <curses.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include "rules.h"

// components
#include "paddle.h"
#include "ball.h"

typedef struct {
    Paddle* paddle;
    Ball* ball;
    WINDOW* scene;
    int ballCount;
    int playerId;
} Pong;

extern Pong pongState;

int ballInPlay();

void setup(int playerId);
void cleanCurses();

void serve();
int gameRunning(int* ch);

void moveUp();
void moveDown();

void ballHandle();
void paddleHandle();

#endif
