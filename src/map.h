#ifndef MAP_H
#define MAP_H

typedef struct
{
    int rows;
    int cols;
    unsigned char *cells;
} Map;

int construct_map(Map *map, FILE *file);

void destruct_map(Map *map);

bool isborder(Map *map, int r, int c, int border);

int start_border(Map *map, int r, int c, int leftright);

#endif
