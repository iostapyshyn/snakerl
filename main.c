#include <SDL2/SDL.h>
#include <stdbool.h>
#include <assert.h>

#include "snakerl.h"
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
                gamestate = QUIT;
                break;
            case SDLK_RETURN:
                /* Selects the level or restarts the game if the game is over. */
                if (gamestate == LOST) {
                    gamestate = RESTART;
                } else gamestate = RUNNING;
                break;
            case SDLK_p:
                /* Pause. */
                if (gamestate == RUNNING)
                    gamestate = PAUSE;
                else if (gamestate == PAUSE)
                    gamestate = RUNNING;
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
                if (gamestate == MENU)
                    level_selection--;
                else newdir = UP;
                break;
            case SDLK_j:
            case SDLK_DOWN:
                if (gamestate == MENU)
                    level_selection++;
                else newdir = DOWN;
                break;
            case SDLK_l:
            case SDLK_RIGHT:
                if (gamestate == RUNNING)
                    newdir = RIGHT;
                break;
            case SDLK_h:
            case SDLK_LEFT:
                if (gamestate == RUNNING)
                    newdir = LEFT;
                break;

            }

            break;
        case SDL_QUIT:
            gamestate = QUIT;
            break;
        }
    }

    /* Update the direction only once per eventpoll. */
    if (newdir != DIRECTION_NOVALUE)
        try_direction(newdir);
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 0;
    }

    ui_cols = 50;
    ui_rows = 25;

    const char *font = argc > 1 ? argv[1] : default_font;

    if (!ui_init(title, font, &ui_cols, &ui_rows))
        return 1;

    ui_effects.crt = true;

    loop();

    ui_quit();
    return 0;
}
