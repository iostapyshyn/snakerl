#include <SDL2/SDL.h>
#include <stdbool.h>
#include <assert.h>

#include "game.h"
#include "const.h"

void eventpoll() {
    direction newdir = DIRECTION_NOVALUE;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {

        case SDL_KEYUP:
            switch (e.key.keysym.sym) {

            case SDLK_q:
                if ((e.key.keysym.mod & KMOD_LSHIFT) != KMOD_LSHIFT)
                    break;
            case SDLK_ESCAPE:
                /* Always exits the game. */
                g.state = QUIT;
                break;
            case SDLK_RETURN:
                if (g.state == MENU) {
                    g.state = RUNNING;
                } else if (g.state == LOST) {
                    g.state = INIT;
                }

                break;
            case SDLK_p:
                if (g.state == PAUSE) {
                    g.state = RUNNING;
                } else if (g.state == RUNNING) {
                    g.state = PAUSE;
                }

                break;
            case SDLK_c:
                /* Disable CRT effect. */
                ui_effects.crt = !ui_effects.crt;
                break;
            }

            break;
        case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {

            case SDLK_k:
            case SDLK_UP:
                if (g.state == MENU)
                    g.level--;

                newdir = UP;
                break;
            case SDLK_j:
            case SDLK_DOWN:
                if (g.state == MENU)
                    g.level++;

                newdir = DOWN;
                break;
            case SDLK_l:
            case SDLK_RIGHT:
                newdir = RIGHT;
                break;
            case SDLK_h:
            case SDLK_LEFT:
                newdir = LEFT;
                break;

            }

            break;

        case SDL_QUIT:
            g.state = QUIT;
            break;
        }
    }

    /* Update the direction only once per eventpoll. */
    if (newdir != DIRECTION_NOVALUE && (g.state == RUNNING || g.state == PAUSE)) {
        g.state = RUNNING;
        game_setdirection(newdir);
    }
}

void draw(void) {
    ui_clear();

    if (g.state == MENU) {
        const int menu_x = ui_cols/2 - ARR_SIZE(menu_str)/2;
        const int menu_y = ui_rows/2 - nlevels/2;

        /* Present the level selection menu. */
        ui_setfg(color_fg);
        ui_putstr(menu_x, menu_y, menu_str);
        for (int i = 0; i < nlevels; i++) {
            if (i == g.level) {
                /* Show current selection with color and arrow. */
                ui_setfg(color_message);
                ui_putstr(menu_x - 3, menu_y + 1 + i, "->");
            } else ui_setfg(color_fg);
            ui_putstr(menu_x, menu_y + 1 + i, levels[i].desc);
        }
    } else {
        ui_clear();
        ui_setfg(color_fg);

        /* Draw the snake. The symbol selection algorithm chooses appropriate symbol for turns,
         * see segment_symbol(int) in const.c. */
        for (int i = g.snake.len-1; i >= 0; i--) {
            ui_putch(g.snake.seg[i].x, g.snake.seg[i].y, segment_symbol(i));
        }

        /* Display food. */
        ui_putch(g.food.x, g.food.y, food_symbol);

        /* Draw game over and pause messages on top, keeping the snake as the background. */
        if (g.state == LOST) {
            ui_setfg(color_message);
            ui_putstr(ui_cols/2-ARR_SIZE(lost_str)/2, ui_rows/2, lost_str);
        } else if (g.state == PAUSE) {
            ui_setfg(color_message);
            ui_putstr(ui_cols/2-ARR_SIZE(pause_str)/2, ui_rows/2, pause_str);
        }

    }

    ui_present();
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 0;
    }

    const char *font = argc > 1 ? argv[1] : default_font;

    if (!ui_init(title, font, UI_COLS, UI_ROWS))
        return 1;

    ui_effects.crt = true;

    game_run(eventpoll, draw);

    ui_quit();

    SDL_Quit();
    return 0;
}
