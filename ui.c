#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include "ui.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define BITMAP_ROWS 16
#define BITMAP_COLS 16

#define INDEX_BG 0
#define INDEX_FG 1

int window_rows, window_cols;

typedef struct {
    SDL_Surface *bitmap;
    SDL_Color palette[2];
    int char_w, char_h;
} ui_font;

ui_font font;

SDL_Window *window;
SDL_Surface *screen;

struct effects ui_effects = {
    .crt = false,
    .crt_intensity = 0.15,
};

static void ui_effect_crt(SDL_Surface *screen) {
    double crtk = 1 - ui_effects.crt_intensity;

    SDL_LockSurface(screen);

    Uint32 *pixels = screen->pixels;
    for (int y = 0; y < screen->h; y++) {
        for (int x = 0; x < screen->w; x++) {
            SDL_Color pixel;
            SDL_GetRGBA(pixels[y*screen->w + x], screen->format, &pixel.r, &pixel.g, &pixel.b, &pixel.a);

            SDL_Color out = pixel;

            switch (y % 3) {
            case 0: out.r = crtk*pixel.r; out.g = crtk*pixel.g; break;
            case 1: out.r = crtk*pixel.r; out.b = crtk*pixel.b; break;
            case 2: out.g = crtk*pixel.g; out.b = crtk*pixel.b; break;
            }

            pixels[y*screen->w + x] = SDL_MapRGBA(screen->format, out.r, out.g, out.b, out.a);
        }
    }

    SDL_UnlockSurface(screen);
}

static int ui_loadfont(const char *filename, ui_font *font) {
    const static SDL_Color bg_default = { 255, 255, 255, SDL_ALPHA_OPAQUE };
    const static SDL_Color fg_default = {   0,   0,   0, SDL_ALPHA_OPAQUE };

    int w, h;
    uint8_t *data = stbi_load(filename, &w, &h, NULL, STBI_rgb_alpha);
    if (data == NULL) {
        SDL_Log("Unable to load font %s: %s", filename, stbi_failure_reason());
        return 0;
    }

    font->bitmap = SDL_CreateRGBSurfaceWithFormat(0, w, h, 8, SDL_PIXELFORMAT_INDEX8);

    SDL_LockSurface(font->bitmap);

    Uint32 *pixels = (Uint32 *) data;
    Uint8 *paletteindexes = font->bitmap->pixels;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (pixels[i*w + j] == pixels[0])
                paletteindexes[i*w + j] = INDEX_BG;
            else paletteindexes[i*w + j] = INDEX_FG;
        }
    }

    SDL_UnlockSurface(font->bitmap);

    stbi_image_free(data);

    font->char_w = font->bitmap->w / BITMAP_COLS;
    font->char_h = font->bitmap->h / BITMAP_ROWS;

    font->palette[INDEX_BG] = bg_default;
    font->palette[INDEX_FG] = fg_default;
    SDL_SetPaletteColors(font->bitmap->format->palette, font->palette, 0, 2);

    SDL_Log("%dx%d font %s successfully loaded.", font->char_w, font->char_h, filename);

    return 1;
}

void ui_quit(void) {
    SDL_FreeSurface(font.bitmap);

    if (window != NULL)
        SDL_DestroyWindow(window);

    SDL_Quit();
}

int ui_init(const char *title, const char *filename, int w, int h) {
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 0;
    }

    if (!ui_loadfont(filename, &font)) {
        SDL_Quit();
        return 0;
    }

    window_cols = w;
    window_rows = h;

    int window_w = font.char_w * window_cols;
    int window_h = font.char_h * window_rows;

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_w, window_h, 0);
    if (window == NULL) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        ui_quit();
        return 0;
    }

    screen = SDL_GetWindowSurface(window);

    return 1;
}

ui_color ui_getbg(void) {
    ui_color bg = {
        font.palette[INDEX_BG].r,
        font.palette[INDEX_BG].g,
        font.palette[INDEX_BG].b,
    };
    return bg;
}

ui_color ui_getfg(void) {
    ui_color fg = {
        font.palette[INDEX_FG].r,
        font.palette[INDEX_FG].g,
        font.palette[INDEX_FG].b,
    };
    return fg;
}

void ui_setbg(ui_color color) {
    font.palette[INDEX_BG] = (SDL_Color) { color.r, color.g, color.b, SDL_ALPHA_OPAQUE };
    SDL_SetPaletteColors(font.bitmap->format->palette, font.palette, 0, 2);
}

void ui_setfg(ui_color color) {
    font.palette[INDEX_FG] = (SDL_Color) { color.r, color.g, color.b, SDL_ALPHA_OPAQUE };
    SDL_SetPaletteColors(font.bitmap->format->palette, font.palette, 0, 2);
}

void ui_putch(int x, int y, char symbol) {
    unsigned char c = symbol;
    SDL_Rect srcrect = { (c % BITMAP_COLS) * font.char_w, (c / BITMAP_ROWS) * font.char_h, font.char_w, font.char_h };
    SDL_Rect dstrect = { x * font.char_w, y*font.char_h, font.char_w, font.char_h };

    SDL_BlitSurface(font.bitmap, &srcrect, screen, &dstrect);
}

void ui_putstr(int x, int y, const char *str) {
    for (const char *ptr = str; *ptr != '\0' && x < window_cols; ptr++)
        ui_putch(x+ptr-str, y, *ptr);
}

void ui_clear(void) {
    SDL_FillRect(screen, NULL, SDL_MapRGBA(screen->format,
                                           font.palette[INDEX_BG].r,
                                           font.palette[INDEX_BG].g,
                                           font.palette[INDEX_BG].b,
                                           SDL_ALPHA_OPAQUE));
}

void ui_present(void) {
    if (ui_effects.crt) ui_effect_crt(screen);
    SDL_UpdateWindowSurface(window);
}
