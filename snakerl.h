#ifndef SNAKERL_SNAKERL_H
#define SNAKERL_SNAKERL_H

#include "ui.h"

#define UI_COLS 50
#define UI_ROWS 25

typedef enum {
    DIRECTION_NOVALUE = -1,
    UP = 0, RIGHT, DOWN, LEFT
} direction;

typedef enum {
    MENU, RUNNING, PAUSE, LOST, RESTART, QUIT
} gamestate_t;

extern gamestate_t gamestate;
extern int ui_cols, ui_rows;
extern int level_selection;

void try_direction(direction newdir);
void loop();

#endif
