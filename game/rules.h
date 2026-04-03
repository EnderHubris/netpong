/**
 * preprocessor globals that define global game rules
 */
#ifndef PONG_RULES
#define PONG_RULES

#define GAMEOVER 3

// desired resolution is 70 columns and 20 rows
#define WIDTH 70
#define HEIGHT 20

#define TICKS_PER_SEC 10

#define MAX_VEL_X 1
#define MAX_VEL_Y 1

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

int setTicker(int nMSecs);

#endif
