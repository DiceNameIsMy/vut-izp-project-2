#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifndef MAP_H
#define MAP_H

typedef enum strategy { RIGHT_HAND, LEFT_HAND, SHORTEST } Strategy;

// Do not change
typedef struct map {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

typedef enum border { RIGHT, LEFT, UP, DOWN } Border;

typedef struct mazeStep {
    int8_t row;
    int8_t column;
    struct MazeStep *nextStep;
} MazeStep;

int construct_map( Map *map, FILE *file );

void destruct_map( Map *map );

// Do not change
bool isborder( Map *map, int r, int c, Border border );

// Do not change
// Get the border to attempt to cross on the first step
Border start_border( Map *map, int r, int c, int leftright );

MazeStep *solve_maze( Map *map, int start_row, int start_column,
                      Strategy strategy );

#endif
