//
//  main.c
//  snakerl
//
//  Created by Ilya Ostapyshyn on 12.01.20.
//

#include "SDL.h"
#include "const.h"
#include "game.h"
#include "ui.h"
#include "stb_image.h"

struct {
    SDL_Surface *arrow;
    SDL_Surface *pause;
    SDL_Rect rect;
} button = { 0 };

void *arrow_data;
void *pause_data;

void eventpoll() {
    direction newdir = DIRECTION_NOVALUE;
    int touch_winx, touch_winy;
    
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_FINGERDOWN:
                touch_winx = e.tfinger.x * ui_surface->w;
                touch_winy = e.tfinger.y * ui_surface->h;
                
                if (touch_winx > button.rect.x && touch_winx < button.rect.x+button.rect.w &&
                    touch_winy > button.rect.y && touch_winy < button.rect.y+button.rect.h) {
                    switch (g.state) {
                        case RUNNING:
                            g.state = PAUSE;
                            break;
                        case PAUSE:
                        case MENU:
                            g.state = RUNNING;
                            break;
                        case LOST:
                            g.state = RESTART;
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
    if (g.state == MENU) {
        const int menu_x = ui_cols/2 - ARR_SIZE(menu_str)/2;
        const int menu_y = ui_rows/2 - nlevels/2;
        
        /* Present the level selection menu. */
        ui_clear();
        
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
        
        SDL_UpperBlitScaled(button.arrow, NULL, ui_surface, &button.rect);
        
        ui_present();
        return;
    }
    
    ui_clear();
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
    
    if (g.state == PAUSE || g.state == RUNNING)
        SDL_UpperBlitScaled(button.pause, NULL, ui_surface, &button.rect);
    else SDL_UpperBlitScaled(button.arrow, NULL, ui_surface, &button.rect);
    
    ui_present();
}

int handle_app_events(void *userdata, SDL_Event *event)
{
    switch (event->type)
    {
        case SDL_APP_TERMINATING:
            /* Terminate the app.
             Shut everything down before returning from this function.
             */
            SDL_FreeSurface(button.arrow);
            SDL_FreeSurface(button.pause);
            stbi_image_free(arrow_data);
            stbi_image_free(pause_data);
            
            ui_quit();
            SDL_Quit();
            return 0;
        case SDL_APP_LOWMEMORY:
            /* You will get this when your app is paused and iOS wants more memory.
             Release as much memory as possible.
             */
            return 0;
        case SDL_APP_WILLENTERBACKGROUND:
            /* Prepare your app to go into the background.  Stop loops, etc.
             This gets called when the user hits the home button, or gets a call.
             */
            g.state = PAUSE;
            return 0;
        case SDL_APP_DIDENTERBACKGROUND:
            /* This will get called if the user accepted whatever sent your app to the background.
             If the user got a phone call and canceled it, you'll instead get an SDL_APP_DIDENTERFOREGROUND event and restart your loops.
             When you get this, you have 5 seconds to save all your state or the app will be terminated.
             Your app is NOT active at this point.
             */
            return 0;
        case SDL_APP_WILLENTERFOREGROUND:
            /* This call happens when your app is coming back to the foreground.
             Restore all your state here.
             */
            return 0;
        case SDL_APP_DIDENTERFOREGROUND:
            /* Restart your loops here.
             Your app is interactive and getting CPU again.
             */
            return 0;
        default:
            /* No special processing, add it to the event queue */
            return 1;
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 0;
    }
    
    SDL_SetEventFilter(handle_app_events, NULL);
    
    if (!ui_init(title, default_font, 0, 0))
        return 1;
    ui_effects.crt = true;
    
    int x, y;
    arrow_data = stbi_load("arrow.png", &x, &y, NULL, STBI_rgb_alpha);
    button.arrow = SDL_CreateRGBSurfaceWithFormatFrom(arrow_data, x, y, 32, 4*x, SDL_PIXELFORMAT_RGBA32);
    
    pause_data = stbi_load("pause.png", &x, &y, NULL, STBI_rgb_alpha);
    button.pause = SDL_CreateRGBSurfaceWithFormatFrom(pause_data, x, y, 32, 4*x, SDL_PIXELFORMAT_RGBA32);
    
    button.rect = (SDL_Rect) { ui_surface->w - 16 - x, 9, x, y };
    
    game_run(eventpoll, draw);
    
    SDL_FreeSurface(button.arrow);
    SDL_FreeSurface(button.pause);
    stbi_image_free(arrow_data);
    stbi_image_free(pause_data);
    
    ui_quit();
    SDL_Quit();
    return 0;
}
