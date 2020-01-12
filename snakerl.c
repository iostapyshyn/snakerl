#include <stdbool.h>
#include <assert.h>
#include "snakerl.h"
#include "const.h"

const direction direction_opposite[] = {
    DOWN, LEFT, UP, RIGHT
};

typedef enum {
    EMPTY, SNAKE, WALL, FOOD
} cell_type;

typedef struct {
    int32_t x, y;
} vec2i;

int ui_cols, ui_rows;
int level_selection = 0;
bool force_update = false;
gamestate_t gamestate;

struct {
    size_t len;
    size_t cap;
    vec2i *seg;
    direction dir;
} snake = { 0, 0, NULL };

void eventpoll(void);

vec2i food;

void try_direction(direction newdir) {
    assert(newdir != DIRECTION_NOVALUE);
    /* Validate the direction change:
     * Do not change direction to the opposite because that would result in
     * instant collision and wouldn't make much sense, except for when
     * the snake is 1 tile long. */
    if (snake.len == 1 || direction_opposite[newdir] != snake.dir) {
        /* force_update guarantees instant turns and more pleasant experience. */
        force_update = true;
        snake.dir = newdir;
    }
}

cell_type cell_gettype(vec2i v) {
    if (v.x == food.x && v.y == food.y)
        return FOOD;

    if (v.x >= ui_cols || v.x < 0 ||
        v.y >= ui_rows || v.y < 0) return WALL;

    for (size_t i = 0; i < snake.len; i++) {
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

void game_update() {
    /* The position of the head after the movement. */
    vec2i head = snake.seg[0];
    switch (snake.dir) {
    case UP:    head.y--; break;
    case RIGHT: head.x++; break;
    case DOWN:  head.y++; break;
    case LEFT:  head.x--; break;
    default:
        assert(0);
    }

    /* If the wall collisions are disabled, make a transition to the opposite wall. */
    cell_type head_cell = cell_gettype(head);
    if (head_cell == WALL && levels[level_selection].wall_collisions == false) {
        if (head.x >= ui_cols)
            head.x -= ui_cols;
        if (head.x < 0)
            head.x += ui_cols;

        if (head.y >= ui_rows)
            head.y -= ui_cols;
        if (head.y < 0)
            head.y += ui_rows;

        head_cell = cell_gettype(head);
    }

    /* Check if the game is over. */
    if (head_cell == SNAKE || (levels[level_selection].wall_collisions ? head_cell == WALL : false)) {
        gamestate = LOST;
        return;
    }

    /* Move all the consequent members. */
    vec2i prev = head;
    for (size_t i = 0; i < snake.len; i++) {
        vec2i tmp = snake.seg[i];
        snake.seg[i] = prev;
        prev = tmp;
    }

    if (head_cell == FOOD) {
        snake_push(prev);

        /* Generate new food. Make sure it generates on empty tile. */
        vec2i newfood;
        do {
            newfood = (vec2i) { rand() % ui_cols, rand() % ui_rows };
        } while (cell_gettype(newfood) != EMPTY);
        food = newfood;
    }
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
    snake.dir = rand() % 4;
    food = (vec2i) { rand() % ui_cols, rand() % ui_rows };
    vec2i head = {
        rand() % (ui_cols/2) + ui_cols/4,
        rand() % (ui_rows/2) + ui_rows/4,
    };

    /* Reset the snake length. */
    snake.len = 0;

    /* Allocate (if needed) the snake and add the initial segment. */
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
            if ((signed int) level_selection > ARR_SIZE(levels)-1)
                level_selection = 0;
            else if ((signed int) level_selection < 0)
                level_selection = ARR_SIZE(levels)-1;

            const int menu_x = ui_cols/2 - ARR_SIZE(menu_str)/2;
            const int menu_y = ui_rows/2 - ARR_SIZE(levels)/2;

            /* Present the level selection menu. */
            ui_clear();

            ui_setfg(color_fg);
            ui_putstr(menu_x, menu_y, menu_str);
            for (int i = 0; i < ARR_SIZE(levels); i++) {
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
            /* Update. */
            force_update = false;
            last_ticks = now_ticks;
            game_update();
        }

        ui_clear();
        ui_setfg(color_fg);

        /* Draw the snake. The symbol selection algorithm chooses appropriate symbol for turns,
         * see snake_segment_symbol(int). */
        for (size_t i = 0; i < snake.len; i++) {
            ui_putch(snake.seg[i].x, snake.seg[i].y, snake_segment_symbol(i));
        }

        /* Display food. */
        ui_putch(food.x, food.y, food_symbol);

        /* Draw game over and pause messages on top, keeping the snake as the background. */
        if (gamestate == LOST) {
            ui_setfg(color_message);
            ui_putstr(ui_cols/2-ARR_SIZE(lost_str)/2, ui_rows/2, lost_str);
        } else if (gamestate == PAUSE) {
            ui_setfg(color_message);
            ui_putstr(ui_cols/2-ARR_SIZE(pause_str)/2, ui_rows/2, pause_str);
        }

        ui_present();
    }

    game_free();
}
