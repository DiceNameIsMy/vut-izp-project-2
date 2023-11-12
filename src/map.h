#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef MAP_H
#define MAP_H

typedef enum strategy { RIGHT_HAND = 0, LEFT_HAND = 1, SHORTEST = 2 } Strategy;

/* ## Represents a map of the maze.

cells - each char represents a cell with 3 passages in the maze.

Cell has 3 passages that can have borders that prevent moving through them. It
is represented by an 8 bit integer. 3 rightmost bits are used. 0 specifies the
absence of a border on the side, and 1 its presence.

First bit (0b001) -> border on the left
Second bit (0b010) -> border on the right
Third bit (0b10) -> border above or below depending on the sum of the
coordinates:
    - odd (1 + 2): passage is above
    - even (1 + 1): passage is below

This structure is the part of the project requirements. Do not change.
*/
typedef struct map {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

Map *load_map( FILE *file );

void destruct_map( Map *map );

typedef enum border { RIGHT, LEFT, UP, DOWN, BORDER_COUNT } Border;

// Do not change
bool isborder( Map *map, int r, int c, Border border );

/* Get the border to attempt to cross on the first step

Do not change
*/
Border start_border( Map *map, int r, int c, int leftright );

void solve_maze( Map *map, int r, int c, Strategy strategy );

#endif
