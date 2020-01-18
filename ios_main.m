//
//  main.c
//  snakerl
//
//  Created by Ilya Ostapyshyn on 12.01.20.
//

#include "const.h"
#include "game.h"
#include "ui.h"

#define BUTTON_W 22
#define BUTTON_H 22

SDL_Surface *arrow_tex;
SDL_Surface *pause_tex;
SDL_Surface *retry_tex;
SDL_Surface *continue_tex;
SDL_Rect button_rect;

int app_handle_events(void *userdata, SDL_Event *event);

int app_init() {
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    /* Event filter for the iOS events. */
    SDL_SetEventFilter(app_handle_events, NULL);

    if (!ui_init(title, default_font, 0, 0))
        return 1;
    ui_effects.crt = true;

    /* Load buttons' textures. */
    arrow_tex = ui_loadtexture("res/arrow.png");
    continue_tex = ui_loadtexture("res/continue.png");
    pause_tex = ui_loadtexture("res/pause.png");
    retry_tex = ui_loadtexture("res/retry.png");

    /* Set up the target rectangle area for the button in the top-left corner. */
    button_rect = (SDL_Rect) { ui_surface->w - BUTTON_W, 0, BUTTON_W, BUTTON_H };

    return 0;
}

void app_terminate() {
    SDL_FreeSurface(arrow_tex);
    SDL_FreeSurface(continue_tex);
    SDL_FreeSurface(pause_tex);
    SDL_FreeSurface(retry_tex);

    ui_quit();
    SDL_Quit();
}

int app_handle_events(void *userdata, SDL_Event *event) {
    switch (event->type) {
        case SDL_APP_TERMINATING:
            /* Terminate the app.
             * Implementation of this method has approximately five seconds
             * to perform any tasks and return. If the method does not return
             * before time expires, the system may kill the process altogether. */

            game_quit();
            app_terminate();
            return 0;
        case SDL_APP_LOWMEMORY:
            /* You will get this when your app is paused and iOS wants more memory.
             * Release as much memory as possible. */

            return 0;
        case SDL_APP_WILLENTERBACKGROUND:
            /* Prepare your app to go into the background.  Stop loops, etc.
             This gets called when the user hits the home button, or gets a call. */

            if (g.state == RUNNING)
                g.state = PAUSE;
            return 0;
        case SDL_APP_DIDENTERBACKGROUND:
            /* This will get called if the user accepted whatever sent your app to the background.
             * If the user got a phone call and canceled it, you'll instead get an SDL_APP_DIDENTERFOREGROUND event and restart your loops.
             * When you get this, you have 5 seconds to save all your state or the app will be terminated.
             * Your app is NOT active at this point. */

            return 0;
        case SDL_APP_WILLENTERFOREGROUND:
            /* This call happens when your app is coming back to the foreground.
             * Restore all your state here. */

            return 0;
        case SDL_APP_DIDENTERFOREGROUND:
            /* Restart your loops here.
             * Your app is interactive and getting CPU again. */

            return 0;
        default:
            /* No special processing, add it to the event queue */
            return 1;
    }
}

void eventpoll() {
    direction newdir = DIRECTION_NOVALUE;
    int touch_winx, touch_winy;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_FINGERDOWN:
                touch_winx = e.tfinger.x * ui_surface->w;
                touch_winy = e.tfinger.y * ui_surface->h;

                if (touch_winx > button_rect.x && touch_winx < button_rect.x+button_rect.w &&
                    touch_winy > button_rect.y && touch_winy < button_rect.y+button_rect.h) {
                    switch (g.state) {
                        case RUNNING:
                            g.state = PAUSE;
                            break;
                        case PAUSE:
                        case MENU:
                            g.state = RUNNING;
                            break;
                        case LOST:
                            g.state = INIT;
                            break;
                        default:
                            break;
                    }
                } else if (g.dir == LEFT || g.dir == RIGHT || g.state == MENU) {
                    if (e.tfinger.y < 0.5) {
                        if (g.state == MENU)
                            g.level--;
                        newdir = UP;
                    } else {
                        if (g.state == MENU)
                            g.level++;
                        newdir = DOWN;
                    }
                } else if (g.dir == UP || g.dir == DOWN) {
                    if (e.tfinger.x < 0.5)
                        newdir = LEFT;
                    else newdir = RIGHT;
                }

                break;
            case SDL_QUIT:
                g.state = QUIT;
                break;
        }
    }

    /* Update the direction only once per eventpoll. */
    if (newdir != DIRECTION_NOVALUE && g.state == RUNNING) {
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
        ui_setfg(color_fg);

        /* Draw the snake. The symbol selection algorithm chooses appropriate symbol for turns,
         * see snake_segment_symbol(int). */
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

    switch (g.state) {
        case MENU:
            SDL_BlitSurface(arrow_tex, NULL, ui_surface, &button_rect);
            break;
        case RUNNING:
            SDL_BlitSurface(pause_tex, NULL, ui_surface, &button_rect);
            break;
        case PAUSE:
            SDL_BlitSurface(continue_tex, NULL, ui_surface, &button_rect);
            break;
        case LOST:
            SDL_BlitSurface(retry_tex, NULL, ui_surface, &button_rect);
            break;
        default: break;
    }

    ui_present();
}

int main(int argc, char *argv[]) {
    if (app_init() != 0)
        return 1;

    game_run(eventpoll, draw);

    app_terminate();
    return 0;
}
