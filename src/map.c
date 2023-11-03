// #define NDEBUG

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

#ifndef NDEBUG
#define loginfo(s, ...)                                                        \
    fprintf(stderr, __FILE__ ":%u: " s "\n", __LINE__, __VA_ARGS__)
#else
#define loginfo(s, ...)
#endif

int load_map_size(Map *map, FILE *file) {
    int rows;
    int columns;

    int r = fscanf(file, "%i %i\n", &rows, &columns);

    bool loaded_both_items = r == 2;

    if (!loaded_both_items || rows < 1 || columns < 1)
        return 1;

    map->rows = rows;
    map->cols = columns;

    return 0;
}

int load_map_cells(Map *map, FILE *file) {
    int row = 1;
    int column = 1;

    char c;
    while ((c = fgetc(file)) != EOF) {
        if (c == ' ') {
            continue;
        } else if (c == '\n') {
            bool all_columns_set = (column - 1) == map->cols;
            if (!all_columns_set) {
                loginfo("tried moving to a next row %i->%i but not all columns "
                        "were set (%i out of %i)",
                        row, row + 1, column - 1, map->cols);
                return 1;
            }

            loginfo("moving to the next line %i->%i", row, row + 1);
            column = 1;
            row++;
            continue;
        }

        bool is_valid_cell = c >= '0' && c <= '7';
        if (!is_valid_cell) {
            loginfo("encountered char `%c` which is not a valid number", c);
            return 1;
        }

        int cell = c - '0';
        int cell_idx = (row * map->cols) + column;

        map->cells[cell_idx] = cell;
        column++;
    }
    return 0;
}

bool has_left_border(int cell) { return ((cell ^ 0b111) & 0b001) == 0b001; }
bool has_right_border(int cell) { return ((cell ^ 0b111) & 0b010) == 0b010; }
bool has_updown_border(int cell) { return ((cell ^ 0b111) & 0b100) == 0b100; }

bool check_right_border(Map *map, int cell, int r, int c) {
    bool has_right_cell = (c + 1) <= map->cols;
    if (!has_right_cell)
        return true;

    int right_cell_idx = (r * map->cols) + (c + 1);
    int right_cell = map->cells[right_cell_idx];

    if (has_right_border(cell) ^ has_left_border(right_cell)) {
        loginfo("map cell at %ix%i with value `%i` is in a mismatch with its "
                "right cell `%i`",
                r, c, cell, right_cell);
        return false;
    }
    return true;
}

bool check_down_border(Map *map, int cell, int r, int c) {
    bool cell_goes_down = ((r + c) & 0b1) == 1;
    if (!cell_goes_down)
        return true;

    bool has_down_cell = (r + 1) <= map->rows;
    if (!has_down_cell)
        return true;

    bool row_cell = map->cells[(r + 1) * c];

    if (has_updown_border(cell) ^ has_updown_border(row_cell)) {
        loginfo("cell at %ix%i with value `%i` is in a mismatch with a cell "
                "beneath it with value `%i`",
                r, c, cell, row_cell);
        return false;
    }

    return true;
}

int get_cell(Map *map, int r, int c) { return map->cells[(r * map->cols) + c]; }

bool check_cell_valid(Map *map, int r, int c) {
    int cell = get_cell(map, r, c);

    bool right_border_valid = check_right_border(map, cell, r, c);
    bool down_border_valid = check_down_border(map, cell, r, c);

    if (!right_border_valid || !down_border_valid) {
        return false;
    }

    return true;
}

bool check_map_valid(Map *map) {
    for (int row_idx = 1; row_idx <= map->rows; row_idx++) {
        for (int col_idx = 1; col_idx <= map->cols; col_idx++) {
            if (!check_cell_valid(map, row_idx, col_idx)) {
                loginfo("map cell %ix%i is invalid", row_idx, col_idx);
                return false;
            }
        }
    }
    return true;
}

int construct_map(Map *map, FILE *file) {
    if (load_map_size(map, file) != 0)
        return 1;

    map->cells = malloc(map->rows * map->cols);

    if (load_map_cells(map, file) != 0)
        return 1;

    if (!check_map_valid(map))
        return 1;

    return 0;
}

void destruct_map(Map *map) { free(map->cells); }

bool isborder(Map *map, int r, int c, int border) {
    if (border == 0b01) {
        return has_left_border(get_cell(map, r, c));
    } else if (border == 0b10) {
        return has_right_border(get_cell(map, r, c));
    } else if (border == 0b11) {
        return has_updown_border(get_cell(map, r, c));
    }

    loginfo("invalid border value `%i`", border);
    return false;
}

int start_border(Map *map, int r, int c, Strategy leftright);
