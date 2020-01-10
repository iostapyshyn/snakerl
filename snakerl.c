#include <SDL2/SDL.h>
#include <stdbool.h>
#include <assert.h>

#include "snakerl.h"

static enum {
    MENU, RUNNING, PAUSE, LOST, RESTART, QUIT
} gamestate;

int level_selection = 0;
bool force_update = false;
enum direction direction;

struct {
    size_t len;
    size_t cap;
    vec2i *seg;
} snake = { 0, 0, NULL };

vec2i food;

void try_direction(enum direction newdir) {
    assert(direction != DIRECTION_NOVALUE);
    /* Validate the direction change:
     * Do not change direction to the opposite because that would result in
     * instant collision and wouldn't make much sense, except for when
     * the snake is 1 tile long. */
    if (snake.len == 1 || direction_opposite[newdir] != direction) {
        /* force_update guarantees instant turns and more pleasurable experience. */
        force_update = true;
        direction = newdir;
    }
}

void eventpoll() {
    enum direction newdirection = DIRECTION_NOVALUE;

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
                /* Exits the menu or leaves the game if the game is over. */
                if (gamestate == LOST) {
                    gamestate = RESTART;
                } else gamestate = RUNNING;
                break;
            case SDLK_p:
                if (gamestate == RUNNING)
                    gamestate = PAUSE;
                else if (gamestate == PAUSE)
                    gamestate = RUNNING;
                break;
            case SDLK_c:
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
                else newdirection = UP;
                break;
            case SDLK_j:
            case SDLK_DOWN:
                if (gamestate == MENU)
                    level_selection++;
                else newdirection = DOWN;
                break;
            case SDLK_l:
            case SDLK_RIGHT:
                newdirection = RIGHT;
                break;
            case SDLK_h:
            case SDLK_LEFT:
                newdirection = LEFT;
                break;

            }

            break;
        case SDL_QUIT:
            gamestate = QUIT;
            break;
        }
    }

    if (newdirection != DIRECTION_NOVALUE)
        try_direction(newdirection);
}

enum celltype get_celltype(vec2i v) {
    if (v.x == food.x && v.y == food.y)
        return FOOD;

    if (v.x >= UI_COLS || v.x < 0 ||
        v.y >= UI_ROWS || v.y < 0) return WALL;

    for (int i = 0; i < snake.len; i++) {
        if (v.x == snake.seg[i].x && v.y == snake.seg[i].y)
            return SNAKE;
    }

    return EMPTY;
}

void snake_push(vec2i segment) {
    /* Allocate/reallocate if needed. */
    if (snake.len >= snake.cap) {
        snake.cap = snake.cap == 0 ? 64 : snake.cap * 2;
        snake.seg = realloc(snake.seg, snake.cap * sizeof(vec2i));
    }

    snake.seg[snake.len++] = segment;
}

int game_update(enum direction dir) {
    /* The position of the head after the movement. */
    vec2i head = snake.seg[0];
    switch (dir) {
    case UP:    head.y--; break;
    case RIGHT: head.x++; break;
    case DOWN:  head.y++; break;
    case LEFT:  head.x--; break;
    default:
        assert(0);
    }

    /* If the wall collisions are disabled, make a transition to the opposite wall. */
    enum celltype head_cell = get_celltype(head);
    if (head_cell == WALL && levels[level_selection].wall_collisions == false) {
        if (head.x >= UI_COLS)
            head.x -= UI_COLS;
        if (head.x < 0)
            head.x += UI_COLS;

        if (head.y >= UI_ROWS)
            head.y -= UI_COLS;
        if (head.y < 0)
            head.y += UI_ROWS;

        head_cell = get_celltype(head);
    }

    /* Check for collisions and return 0 if dead. */
    if (head_cell == SNAKE || (levels[level_selection].wall_collisions ? head_cell == WALL : false))
        return 0;

    /* Move all the consequent members. */
    vec2i prev = head;
    for (int i = 0; i < snake.len; i++) {
        vec2i tmp = snake.seg[i];
        snake.seg[i] = prev;
        prev = tmp;
    }

    if (head_cell == FOOD) {
        snake_push(prev);

        /* Generate new food. Make sure it generates on empty tile. */
        vec2i newfood;
        do {
            newfood = (vec2i) { rand() % UI_COLS, rand() % UI_ROWS };
        } while (get_celltype(newfood) != EMPTY);
        food = newfood;
    }

    return 1;
}

char snake_segment_symbol(size_t i) {
    char symbol = '?';
    if (i == 0) { /* Head */
        symbol = head_symbol;
    } else if (i == snake.len-1) { /* Last segment of the tail */
        if (snake.seg[i].x - snake.seg[i-1].x == 0) symbol = seg_v;
        if (snake.seg[i].y - snake.seg[i-1].y == 0) symbol = seg_h;
    } else {
        vec2i AB = { snake.seg[i].x - snake.seg[i-1].x, snake.seg[i].y - snake.seg[i-1].y };
        vec2i BC = { snake.seg[i+1].x - snake.seg[i].x, snake.seg[i+1].y - snake.seg[i].y };
        /* Moving away from head: segment A(i-1) to segment C(i+1) through B(i).
         * The second condition of each if branch is used to ensure proper transitions
         * to the opposite part of the screen (for example on EASY level). */
        if (AB.y == -1 || AB.y > 1) { /* up */
            if (BC.x == 0) /* up */
                symbol = seg_v; /* vertical */
            else if (BC.x == -1 || BC.x >  1)
                symbol = seg_ul; /* up and left */
            else if (BC.x ==  1 || BC.x < -1)
                symbol = seg_ur; /* up and right */
        } else if (AB.y == 1 || AB.y < -1) { /* down */
            if (BC.x == 0) /* down */
                symbol = seg_v; /* vertical */
            else if (BC.x == -1 || BC.x >  1)
                symbol = seg_dl; /* down and left */
            else if (BC.x ==  1 || BC.x < -1)
                symbol = seg_dr; /* down and right */
        } else if (AB.x == -1 || AB.x > 1) { /* left */
            if (BC.y == 0) /* left */
                symbol = seg_h; /* horizontal */
            else if (BC.y == -1 || BC.y >  1)
                symbol = seg_lu; /* left and up */
            else if (BC.y ==  1 || BC.y < -1)
                symbol = seg_ld; /* left and down */
        } else if (AB.x == 1 || AB.x < -1) { /* right */
            if (BC.y == 0) /* right */
                symbol = seg_h; /* horizontal */
            else if (BC.y == -1 || BC.y >  1)
                symbol = seg_ru; /* right and up */
            else if (BC.y ==  1 || BC.y < -1)
                symbol = seg_rd; /* right and down */
        }
    }
    return symbol;
}

void game_init() {
    srand(time(NULL));

    /* Randomize initial parameters. */
    direction = rand() % 4;
    food = (vec2i) { rand() % UI_COLS, rand() % UI_ROWS };
    vec2i head = {
        rand() % (UI_COLS/2) + UI_COLS/4,
        rand() % (UI_ROWS/2) + UI_ROWS/4,
    };

    /* Allocate the snake and add the initial segment. */
    snake_push(head);

    gamestate = MENU;
}

void game_free(void) {
    free(snake.seg);
}

void loop() {
    game_init();

    uint32_t last_ticks = SDL_GetTicks();
    while (gamestate != QUIT) {
        eventpoll();

        if (gamestate == RESTART) {
            game_init();
            continue;
        }

        if (gamestate == MENU) {
            if ((signed int) level_selection > LEVEL_HARD)
                level_selection = LEVEL_EASY;
            else if ((signed int) level_selection < LEVEL_EASY)
                level_selection = LEVEL_HARD;

            const int menu_x = UI_COLS/2 - strlen(menu_str)/2;
            const int menu_y = UI_ROWS/2 - sizeof(levels)/sizeof(*levels);

            /* Present the level selection menu. */
            ui_clear();

            ui_setfg(color_fg);
            ui_putstr(menu_x, menu_y, menu_str);
            for (int i = 0; i < 3; i++) {
                if (i == level_selection) {
                    /* Show current selection with color and arrow. */
                    ui_setfg(color_message);
                    ui_putstr(menu_x - 3, menu_y + 1 + i, "->");
                } else ui_setfg(color_fg);
                ui_putstr(menu_x, menu_y + 1 + i, levels[i].desc);
            }

            ui_present();
            continue;
        }

        uint32_t now_ticks = SDL_GetTicks();
        if (gamestate == RUNNING &&
            (force_update || now_ticks - last_ticks > levels[level_selection].update_ms)) {

            force_update = false;
            last_ticks = now_ticks;
            if (!game_update(direction)) {
                gamestate = LOST;
            }
        }

        ui_clear();
        ui_setfg(color_fg);

        /* Draw the snake. The symbol selection algorithm is too long to put it here. */
        for (int i = 0; i < snake.len; i++) {
            ui_putch(snake.seg[i].x, snake.seg[i].y, snake_segment_symbol(i));
        }

        ui_putch(food.x, food.y, food_symbol);

        if (gamestate == LOST) {
            ui_setfg(color_message);
            ui_putstr(UI_COLS/2-strlen(lost_str)/2, UI_ROWS/2, lost_str);
        } else if (gamestate == PAUSE) {
            ui_setfg(color_message);
            ui_putstr(UI_COLS/2-strlen(pause_str)/2, UI_ROWS/2, pause_str);
        }

        ui_present();
    }

    /* Since the snake array is dynamically allocated: */
    game_free();
}

int main(int argc, char *argv[]) {
    const char *font = argc > 1 ? argv[1] : default_font;

    if (!ui_init(title, font, UI_COLS, UI_ROWS))
        return 1;

    ui_effects.crt = true;

    loop();

    ui_quit();
    return 0;
}
