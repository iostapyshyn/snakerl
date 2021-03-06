#include <stdbool.h>
#include <assert.h>
#include <time.h>

#include "game.h"
#include "const.h"

typedef enum {
    EMPTY, SNAKE, WALL, FOOD
} cell_type;

/* The g.state is RESTART by default. */
game_data g = { 0 };

static bool force_update = false;

void game_setdirection(direction newdir) {
    if (((newdir == UP || newdir == DOWN) && (g.dir == LEFT || g.dir == RIGHT)) ||
        ((newdir == LEFT || newdir == RIGHT) && (g.dir == UP || g.dir == DOWN))) {
        force_update = true;
        g.dir = newdir;
    }
}

static cell_type cell_gettype(vec2i v) {
    if (v.x == g.food.x && v.y == g.food.y)
        return FOOD;

    if (v.x >= ui_cols || v.x < 0 ||
        v.y >= ui_rows || v.y < 0) return WALL;

    for (size_t i = 1; i < g.snake.len; i++) {
        if (v.x == g.snake.seg[i].x && v.y == g.snake.seg[i].y)
            return SNAKE;
    }

    return EMPTY;
}

static void snake_push(vec2i segment) {
    /* Allocate/reallocate if needed. */
    if (g.snake.len >= g.snake.cap) {
        g.snake.cap = g.snake.cap == 0 ? 64 : g.snake.cap * 2;
        g.snake.seg = realloc(g.snake.seg, g.snake.cap * sizeof(vec2i));
    }

    g.snake.seg[g.snake.len++] = segment;
}

static void game_init() {
    srand(time(NULL));

    /* Randomize initial parameters. */
    g.dir = rand() % 4;
    g.food = (vec2i) { rand() % ui_cols, rand() % ui_rows };

    /* Reset the snake length. */
    g.snake.len = 0;

    vec2i seg = {
        rand() % (ui_cols/2) + ui_cols/4,
        rand() % (ui_rows/2) + ui_rows/4,
    };

    /* Allocate (if needed) the snake and add the initial segment. */
    snake_push(seg);

    /* Generate a tail in opposite direction of the initial movement. */
    switch (g.dir) {
    case UP:    seg.y++; break;
    case RIGHT: seg.x--; break;
    case DOWN:  seg.y--; break;
    case LEFT:  seg.x++; break;
    default: assert(0);
    }

    snake_push(seg);

    g.state = MENU;
}

static void game_update() {
    /* Move the tail part of the snake. */
    vec2i last = g.snake.seg[g.snake.len-1];
    for (size_t i = g.snake.len-1; i > 0; i--) {
        g.snake.seg[i] = g.snake.seg[i-1];
    }

    /* The position of the head after the movement. */
    switch (g.dir) {
    case UP:    g.snake.seg[0].y--; break;
    case RIGHT: g.snake.seg[0].x++; break;
    case DOWN:  g.snake.seg[0].y++; break;
    case LEFT:  g.snake.seg[0].x--; break;
    default:
        assert(0);
    }

    /* If the wall collisions are disabled, make a transition to the opposite wall. */
    cell_type head_cell = cell_gettype(g.snake.seg[0]);
    if (head_cell == WALL && levels[g.level].wall_collisions == false) {
        if (g.snake.seg[0].x >= ui_cols || g.snake.seg[0].x < 0) {
            g.snake.seg[0].x += ui_cols;
            g.snake.seg[0].x %= ui_cols;
        }

        if (g.snake.seg[0].y >= ui_rows || g.snake.seg[0].y < 0) {
            g.snake.seg[0].y += ui_rows;
            g.snake.seg[0].y %= ui_rows;
        }

        head_cell = cell_gettype(g.snake.seg[0]);
    }

    /* Check if the game is over. */
    if (head_cell == SNAKE || (levels[g.level].wall_collisions ? head_cell == WALL : false)) {
        g.state = LOST;
        return;
    }

    if (head_cell == FOOD) {
        snake_push(last);

        /* Generate new food. Make sure it generates on empty tile. */
        vec2i newfood;
        do {
            newfood = (vec2i) { rand() % ui_cols, rand() % ui_rows };
        } while (cell_gettype(newfood) != EMPTY);
        g.food = newfood;
    }
}

void game_quit(void) {
    g.state = QUIT;
    free(g.snake.seg);
}

void game_run(void (*eventpoll)(void), void (*draw)(void)) {
    uint32_t last_ticks = SDL_GetTicks();
    while (g.state != QUIT) {
        if (g.state == INIT) {
            game_init();
            continue;
        }
        
        eventpoll();

        if (g.state == MENU) {
            if ((signed int) g.level > nlevels-1)
                g.level = 0;
            else if ((signed int) g.level < 0)
                g.level = nlevels-1;
        } else if (g.state == RUNNING) {
            uint32_t now_ticks = SDL_GetTicks();
            if (force_update || now_ticks - last_ticks > levels[g.level].update_ms) {
                /* Update. */
                force_update = false;
                last_ticks = now_ticks;
                game_update();
            }
        }

        draw();
    }

    game_quit();
}
