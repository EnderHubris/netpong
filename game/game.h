#ifndef PONG_GAME
#define PONG_GAME

#include <curses.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>
#include <sys/time.h>
#include <sys/select.h>

#include "rules.h"

// components
#include "paddle.h"
#include "ball.h"

#include "../utils.h"

typedef struct {
    Paddle* paddle;
    Ball* ball;
    
    WINDOW* scene;
    WINDOW* scoreWind;

    int ballCount;
    int playerId;
    int socket_fd;
    int running;
} Pong;

extern Pong pongState;
extern char scoreText[64];

int ballInPlay();

void setup(int playerId, char* hostStr, int port);
void cleanCurses();
void closeGame();

void serve(int playerId);
int gameRunning(int* ch);
void checkForChange();

void moveUp();
void moveDown();

void ballHandle();
void paddleHandle();

#endif
