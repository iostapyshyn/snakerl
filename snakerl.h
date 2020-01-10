#ifndef SNAKERL_SNAKERL_H
#define SNAKERL_SNAKERL_H

#include "ui.h"

#define UI_COLS 50
#define UI_ROWS 25

typedef struct {
    int32_t x, y;
} vec2i;

enum celltype {
    EMPTY, SNAKE, WALL, FOOD
};

enum direction {
    DIRECTION_NOVALUE = -1,
    UP = 0, RIGHT, DOWN, LEFT
};

const enum direction direction_opposite[] = {
    DOWN, LEFT, UP, RIGHT
};

const char head_symbol = '@';
const char seg_symbol = '#';
const char food_symbol = '\1';

const char seg_v = '\xBA';
const char seg_h = '\xCD';

const char seg_ur = '\xC9';
const char seg_ul = '\xBB';
const char seg_ru = '\xBC';
const char seg_lu = '\xC8';

const char seg_dr = seg_lu;
const char seg_dl = seg_ru;
const char seg_rd = seg_ul;
const char seg_ld = seg_ur;

const ui_color color_bg      = { 255, 255, 255 };
const ui_color color_fg      = {   0,   0,   0 };
const ui_color color_message = { 255,   0,   0 };

const char *pause_str = "*PAUSE*";
const char *lost_str = "Game Over.";
const char *menu_str = "Select the difficulty level:";

enum {
    LEVEL_EASY = 0,
    LEVEL_MEDIUM,
    LEVEL_HARD,
};

const struct level {
    const char *desc;
    unsigned int update_ms;
    bool wall_collisions;
} levels[] = {
    { "EASY", 200, false },
    { "HARD", 100, true },
    { "CHALLENGING", 75, true },
};

#endif
