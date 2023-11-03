#ifndef MAP_H
#define MAP_H

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

typedef enum { RIGHT_HAND, LEFT_HAND, SHORTEST } Strategy;

int construct_map(Map *map, FILE *file);

void destruct_map(Map *map);

int start_border(Map *map, int r, int c, Strategy leftright);

#endif
