#ifndef SNAKERL_CONST_H
#define SNAKERL_CONST_H

#include <stdbool.h>
#include "ui.h"

/* General constants */

#define ARR_SIZE(x) sizeof(x)/sizeof(*x)

static const char *title = "Snake";
static const char *default_font = "fonts/terminus_11x11.bmp";

static const char head_symbol = '@';
static const char seg_symbol = '#';
static const char food_symbol = '\1';

static const char seg_v = '\xBA';
static const char seg_h = '\xCD';

static const char seg_ur = '\xC9';
static const char seg_ul = '\xBB';
static const char seg_ru = '\xBC';
static const char seg_lu = '\xC8';

static const char seg_dr = seg_lu;
static const char seg_dl = seg_ru;
static const char seg_rd = seg_ul;
static const char seg_ld = seg_ur;

static const ui_color color_bg      = { 255, 255, 255 };
static const ui_color color_fg      = {   0,   0,   0 };
static const ui_color color_message = { 255,   0,   0 };

static const char pause_str[] = "*PAUSE*";
static const char lost_str[] = "Game Over.";
static const char menu_str[] = "Select the difficulty level:";

static const struct {
    const char *desc;
    unsigned int update_ms;
    bool wall_collisions;
} levels[] = {
    { "EASY", 200, false },
    { "HARD", 100, true },
    { "CHALLENGING", 75, true },
};

#endif
