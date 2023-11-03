#include <stdio.h>

#ifndef MAP_H
#define MAP_H

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

typedef enum strategy { RIGHT_HAND, LEFT_HAND, SHORTEST } Strategy;

int construct_map( Map *map, FILE *file );

void destruct_map( Map *map );

void solve_maze( Map *map, int start_row, int start_column, Strategy strategy );

#endif
