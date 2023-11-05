// #define NDEBUG

#include "map.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#define loginfo( s, ... ) \
    fprintf( stderr, __FILE__ ":%u: " s "\n", __LINE__, __VA_ARGS__ )
#else
#define loginfo( s, ... )
#endif

typedef enum processCharResult { OK, BAD_MAP, BAD_CELL } ProcessCharResult;

int load_map_size( Map *map, FILE *file ) {
    int rows;
    int columns;

    int r = fscanf( file, "%i %i\n", &rows, &columns );

    bool loaded_both_items = r == 2;

    if ( !loaded_both_items || rows < 1 || columns < 1 )
        return 1;

    map->rows = rows;
    map->cols = columns;

    return 0;
}

int to_cell( char c ) {
    bool is_valid_cell = c >= '0' && c <= '7';
    if ( !is_valid_cell )
        return -1;

    return c - '0';
}

ProcessCharResult process_char( Map *map, char c, int *row_ptr, int *col_ptr ) {
    int row = *row_ptr;
    int column = *col_ptr;

    if ( c == ' ' ) {
        return OK;
    } else if ( c == '\n' ) {
        bool all_columns_set = ( column - 1 ) == map->cols;
        if ( !all_columns_set )
            return BAD_MAP;

        loginfo( "moving to the next line %i->%i", row, row + 1 );

        ( *row_ptr )++;
        *col_ptr = 1;

        return OK;
    }

    int cell = to_cell( c );
    if ( cell == -1 ) {
        return BAD_CELL;
    }

    int cell_idx = ( row * map->cols ) + column;

    loginfo( "adding cell: %i", cell );
    map->cells[ cell_idx ] = cell;
    ( *col_ptr )++;

    return OK;
}

int load_map_cells( Map *map, FILE *file ) {
    int row = 1;
    int column = 1;

    char c;
    while ( ( c = fgetc( file ) ) != EOF ) {
        ProcessCharResult r = process_char( map, c, &row, &column );

        if ( r == BAD_MAP ) {
            loginfo( "tried moving to a next row %i->%i but not all columns "
                     "were set (%i out of %i)",
                     row, row + 1, column - 1, map->cols );
            return 1;
        } else if ( r == BAD_CELL ) {
            loginfo( "encountered char `%c` which is not a valid number", c );
            return 1;
        }
    }
    return 0;
}

bool has_left_border( int cell ) {
    return ( ( cell ^ 0b111 ) & 0b001 ) == 0b001;
}
bool has_right_border( int cell ) {
    return ( ( cell ^ 0b111 ) & 0b010 ) == 0b010;
}
bool has_updown_border( int cell ) {
    return ( ( cell ^ 0b111 ) & 0b100 ) == 0b100;
}

bool check_right_border( Map *map, int cell, int r, int c ) {
    bool has_right_cell = ( c + 1 ) <= map->cols;
    if ( !has_right_cell )
        return true;

    int right_cell_idx = ( r * map->cols ) + ( c + 1 );
    int right_cell = map->cells[ right_cell_idx ];

    if ( has_right_border( cell ) ^ has_left_border( right_cell ) ) {
        loginfo( "map cell at %ix%i with value `%i` is in a mismatch with its "
                 "right cell `%i`",
                 r, c, cell, right_cell );
        return false;
    }
    return true;
}

bool check_down_border( Map *map, int cell, int r, int c ) {
    bool cell_goes_down = ( ( r + c ) & 0b1 ) == 1;
    if ( !cell_goes_down )
        return true;

    bool has_down_cell = ( r + 1 ) <= map->rows;
    if ( !has_down_cell )
        return true;

    bool row_cell = map->cells[ ( r + 1 ) * c ];

    if ( has_updown_border( cell ) ^ has_updown_border( row_cell ) ) {
        loginfo( "cell at %ix%i with value `%i` is in a mismatch with a cell "
                 "beneath it with value `%i`",
                 r, c, cell, row_cell );
        return false;
    }

    return true;
}

int get_cell( Map *map, int r, int c ) {
    return map->cells[ ( r * map->cols ) + c ];
}

bool check_cell_valid( Map *map, int r, int c ) {
    int cell = get_cell( map, r, c );

    bool right_border_valid = check_right_border( map, cell, r, c );
    bool down_border_valid = check_down_border( map, cell, r, c );

    if ( !right_border_valid || !down_border_valid ) {
        return false;
    }

    return true;
}

bool check_map_valid( Map *map ) {
    for ( int row_idx = 1; row_idx <= map->rows; row_idx++ ) {
        for ( int col_idx = 1; col_idx <= map->cols; col_idx++ ) {
            if ( !check_cell_valid( map, row_idx, col_idx ) ) {
                loginfo( "map cell %ix%i is invalid", row_idx, col_idx );
                return false;
            }
        }
    }
    return true;
}

int construct_map( Map *map, FILE *file ) {
    if ( load_map_size( map, file ) != 0 )
        return 1;

    map->cells = malloc( map->rows * map->cols );

    if ( load_map_cells( map, file ) != 0 )
        return 1;

    if ( !check_map_valid( map ) )
        return 1;

    return 0;
}

void destruct_map( Map *map ) { free( map->cells ); }

bool isborder( Map *map, int r, int c, Border border ) {
    int cell = get_cell( map, r, c );
    if ( border == RIGHT ) {
        return has_right_border( cell );
    } else if ( border == LEFT ) {
        return has_left_border( cell );
    } else if ( border == UP || border == DOWN ) {
        return has_updown_border( cell );
    }

    loginfo( "invalid border value `%i`", border );
    return false;
}
