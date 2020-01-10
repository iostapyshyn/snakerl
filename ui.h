#ifndef SNAKERL_UI_H
#define SNAKERL_UI_H

#include <SDL2/SDL.h>
#include <stdbool.h>

typedef struct {
    Uint8 r, g, b;
} ui_color;

struct effects {
    bool crt;
    double crt_intensity;
};

extern struct effects ui_effects;

int ui_init(const char *title, const char *filename, int w, int h);
void ui_quit(void);

ui_color ui_getbg(void);
void ui_setbg(ui_color);
ui_color ui_getfg(void);
void ui_setfg(ui_color);

void ui_putch(int x, int y, char c);
void ui_putstr(int x, int y, const char *str);

void ui_clear(void);
void ui_present(void);

#endif
