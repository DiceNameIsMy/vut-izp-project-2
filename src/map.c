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

/*

MAP INITIALIZATION & DESTRUCTION

*/

int read_map_size( Map *map, FILE *file ) {
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

typedef enum processCharResult { OK, BAD_MAP, BAD_CELL } ProcessCharResult;

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

int read_map_cells( Map *map, FILE *file ) {
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

bool has_passage_above( int r, int c ) { return ( ( ( r + c ) & 1 ) == 0 ); }

bool check_down_border( Map *map, int cell, int r, int c ) {
    bool cell_goes_up = has_passage_above( r, c );
    if ( cell_goes_up )
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

/* check vailidy of every cell's right & down border starting from top left
 */
bool is_map_valid( Map *map ) {
    for ( int r = 1; r <= map->rows; r++ ) {
        for ( int c = 1; c <= map->cols; c++ ) {
            if ( !check_cell_valid( map, r, c ) ) {
                loginfo( "map cell %ix%i is invalid", r, c );
                return false;
            }
        }
    }
    return true;
}

Map *allocate_map() {
    Map *map = malloc( sizeof( Map ) );
    if ( map == NULL )
        return NULL;

    map->rows = 0;
    map->cols = 0;

    map->cells = malloc( map->rows * map->cols );
    if ( map->cells == NULL )
        return NULL;

    return map;
}

void destruct_map( Map *map ) {
    free( map->cells );
    free( map );
}

Map *load_map( FILE *file ) {
    Map *map = allocate_map();
    if ( map == NULL ) {
        return NULL;
    }

    if ( read_map_size( map, file ) != 0 ) {
        destruct_map( map );
        return NULL;
    }

    if ( read_map_cells( map, file ) != 0 ) {
        destruct_map( map );
        return NULL;
    }

    if ( !is_map_valid( map ) ) {
        destruct_map( map );
        return NULL;
    }

    return map;
}

/*

STARTING POINT RESOLVEMENT

*/

bool out_of_bounds( Map *m, int r, int c ) {
    return ( r < 1 || r > m->rows || c < 1 || c > m->cols );
}

/* Resolve to what passage to look at next
- first accessor is a strategy: LEFT_HAND / RIGHT_HAND
- second accessor: Border where from player came
- third accessor: Whether player's current cell has a passage UP
 */
Border next_step_ruleset[ 2 ][ BORDER_COUNT ][ 2 ] = {
    [RIGHT_HAND] = { [LEFT] = { DOWN, RIGHT },
                     [RIGHT] = { LEFT, UP },
                     [UP] = { LEFT, LEFT },
                     [DOWN] = { RIGHT, RIGHT } },
    [LEFT_HAND] = { [LEFT] = { RIGHT, UP },
                    [RIGHT] = { DOWN, LEFT },
                    [UP] = { RIGHT, RIGHT },
                    [DOWN] = { LEFT, LEFT } } };

Border resolve_came_from( Map *map, int r, int c ) {
    if ( c == 1 )
        return LEFT;
    else if ( c == map->cols )
        return RIGHT;
    else if ( r == 1 )
        return UP;
    else if ( r == map->rows )
        return DOWN;

    return -1;
}

Border start_border( Map *map, int r, int c, int leftright ) {
    if ( leftright == SHORTEST ) {
        loginfo( "invalid leftright value %i", leftright );
        return -1;
    }

    Border came_from = resolve_came_from( map, r, c );
    if ( (int)came_from == -1 ) {
        loginfo( "not entering the maze from borders (%ix%i)", r, c );
        return -1;
    }

    bool can_go_up = ( ( ( r + c ) & 1 ) == 0 );

    return next_step_ruleset[ leftright ][ came_from ][ can_go_up ];
}

// void take_step( Map *m, int *r, int *c, Border *from ) {}

// void solve_maze( Map *map, int r, int c, Strategy strategy ) {
//     if ( strategy == SHORTEST ) {
//         loginfo( "not implemented strategy: %i", strategy );
//         return;
//     }

//     Border from = start_border( map, r, c, strategy );

//     while ( true ) {
//         if ( out_of_bounds( map, r, c ) ) {
//             return;
//         }
//         printf( "%i, %i\n", r, c );

//         take_step( map, &r, &c, &from );
//     }
// }

/*

NOT GROUPED YET

*/

int move_incr[ BORDER_COUNT ][ 2 ] = {
    [RIGHT] = { 0, 1 },
    [LEFT] = { 0, -1 },
    [UP] = { -1, 0 },
    [DOWN] = { 1, 0 },
};

bool moves_out_of_bounds( Map *m, int r, int c, Border direction ) {
    int moved_r = r + move_incr[ direction ][ 0 ];
    int moved_c = c + move_incr[ direction ][ 1 ];
    return out_of_bounds( m, moved_r, moved_c );
}

bool ( *solve_border_func[ 4 ] )( int ) = {
    has_left_border,
    has_right_border,
    has_updown_border,
    has_updown_border,
};

bool isborder( Map *map, int r, int c, Border border ) {
    int cell = get_cell( map, r, c );
    return ( *solve_border_func[ border ] )( cell );
}
