#include "game.h"
#include "const.h"

const struct level levels[] = {
    { "EASY", 200, false },
    { "HARD", 100, true },
    { "CHALLENGING", 75, true },
};

const int nlevels = ARR_SIZE(levels);

char segment_symbol(size_t i) {
    char symbol = '?';
    if (i == 0) { /* Head */
        symbol = seg_head;
    } else if (i == g.snake.len-1) { /* Last segment of the tail */
        if (g.snake.seg[i].x - g.snake.seg[i-1].x == 0) symbol = seg_v;
        if (g.snake.seg[i].y - g.snake.seg[i-1].y == 0) symbol = seg_h;
    } else {
        vec2i AB = { g.snake.seg[i].x - g.snake.seg[i-1].x, g.snake.seg[i].y - g.snake.seg[i-1].y };
        vec2i BC = { g.snake.seg[i+1].x - g.snake.seg[i].x, g.snake.seg[i+1].y - g.snake.seg[i].y };
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
