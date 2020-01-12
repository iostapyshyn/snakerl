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
                game_state = QUIT;
                break;
            case SDLK_RETURN:
                game_continue();
                break;
            case SDLK_p:
                game_pause();
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
                if (game_state == MENU)
                    game_level--;
                else newdir = UP;
                break;
            case SDLK_j:
            case SDLK_DOWN:
                if (game_state == MENU)
                    game_level++;
                else newdir = DOWN;
                break;
            case SDLK_l:
            case SDLK_RIGHT:
                if (game_state == RUNNING)
                    newdir = RIGHT;
                break;
            case SDLK_h:
            case SDLK_LEFT:
                if (game_state == RUNNING)
                    newdir = LEFT;
                break;

            }

            break;
        case SDL_QUIT:
            game_state = QUIT;
            break;
        }
    }

    /* Update the direction only once per eventpoll. */
    if (newdir != DIRECTION_NOVALUE)
        game_setdirection(newdir);
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

    game_run(eventpoll);

    ui_quit();

    SDL_Quit();
    return 0;
}
