#ifndef SNAKERL_GAME_H
#define SNAKERL_GAME_H

#include "ui.h"

typedef enum {
    DIRECTION_NOVALUE = -1,
    UP = 0, RIGHT, DOWN, LEFT
} direction;

typedef enum {
    MENU, RUNNING, PAUSE, LOST, RESTART, QUIT
} gamestate;

extern gamestate game_state;
extern int game_level;

void game_setdirection(direction newdir);
void game_continue(void);
void game_pause(void);
void game_run(void (*eventpoll)(void));

#endif
