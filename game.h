#ifndef SNAKERL_GAME_H
#define SNAKERL_GAME_H

#include "ui.h"

typedef struct {
    int32_t x, y;
} vec2i;

typedef enum {
    DIRECTION_NOVALUE = -1,
    UP = 0, RIGHT, DOWN, LEFT
} direction;

typedef enum {
    MENU, RUNNING, PAUSE, LOST, RESTART, QUIT
} game_state;

typedef struct {
    game_state state;
    direction dir;
    int level;
    struct {
        size_t len;
        size_t cap;
        vec2i *seg;
    } snake;
    vec2i food;
} game_data;

extern game_data g;

void game_setdirection(direction newdir);
void game_continue(void);
void game_pause(void);
void game_run(void (*eventpoll)(void), void (*present)(void));

char segment_symbol(size_t i);

#endif
