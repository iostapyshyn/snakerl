#ifndef SNAKERL_UI_H
#define SNAKERL_UI_H

#ifdef TARGET_IOS
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

#include <stdbool.h>

#define INDEX_BG 0
#define INDEX_FG 1

typedef struct {
    Uint8 r, g, b;
} ui_color;

struct ui_effects {
    bool crt;
    double crt_intensity;
};

extern struct ui_effects ui_effects;
extern int ui_rows, ui_cols;
extern SDL_Window *ui_window;
extern SDL_Surface *ui_surface;

int ui_init(const char *title, const char *filename, int w, int h);
void ui_quit(void);

void ui_setbg(ui_color);
void ui_setfg(ui_color);

void ui_putch(int x, int y, char c);
void ui_putstr(int x, int y, const char *str);

void ui_clear(void);
void ui_present(void);

#endif
