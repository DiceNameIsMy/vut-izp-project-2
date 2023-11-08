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

    if ( load_map_size( map, file ) != 0 ) {
        destruct_map( map );
        return NULL;
    }

    if ( load_map_cells( map, file ) != 0 ) {
        destruct_map( map );
        return NULL;
    }

    if ( !check_map_valid( map ) ) {
        destruct_map( map );
        return NULL;
    }

    return map;
}

bool ( *border_solvers[ 4 ] )( int ) = {
    has_left_border,
    has_right_border,
    has_updown_border,
    has_updown_border,
};

bool isborder( Map *map, int r, int c, Border border ) {
    int cell = get_cell( map, r, c );
    return ( *border_solvers[ border ] )( cell );
}

bool out_of_bounds( Map *m, int r, int c ) {
    return ( r < 1 || r > m->rows || c < 1 || c > m->cols );
}

int move_map[ BORDER_COUNT ][ 2 ] = {
    { 0, 1 },
    { 0, -1 },
    { -1, 0 },
    { 1, 0 },
};

bool moves_out_of_bounds( Map *m, int r, int c, Border direction ) {
    int moved_r = r + move_map[ direction ][ 0 ];
    int moved_c = c + move_map[ direction ][ 1 ];
    return out_of_bounds( m, moved_r, moved_c );
}

typedef struct entrance {
    bool from_left;
    bool from_right;
    bool from_up;
    bool from_down;
    bool has_passage_above;
} Entrance;

Entrance get_entrance_spec( Map *map, int r, int c ) {
    Entrance e = { .from_left = c == 1,
                   .from_right = c == map->cols,
                   .from_up = r == 1,
                   .from_down = r == map->rows,
                   .has_passage_above = ( ( ( r + c ) & 1 ) == 0 ) };

    loginfo( "entrace: L:%i R:%i U:%i D:%i Passage above:%i", e.from_left,
             e.from_right, e.from_up, e.from_down, e.has_passage_above );

    return e;
}

Border rhand_start_border( Map *map, int r, int c ) {
    Entrance e = get_entrance_spec( map, r, c );

    if ( e.from_left ) {
        if ( !e.has_passage_above ) {
            return DOWN;
        }
        return RIGHT;
    } else if ( e.from_right ) {
        if ( e.has_passage_above ) {
            return UP;
        }
        return LEFT;
    }

    if ( e.from_up ) {
        return LEFT;
    } else if ( e.from_down ) {
        return RIGHT;
    }

    return -1;
}

Border lhand_start_border( Map *map, int r, int c ) {
    Entrance e = get_entrance_spec( map, r, c );

    if ( e.from_left ) {
        if ( e.has_passage_above ) {
            return UP;
        }
        return RIGHT;
    } else if ( e.from_right ) {
        if ( !e.has_passage_above ) {
            return DOWN;
        }
        return LEFT;
    }

    if ( e.from_up ) {
        return RIGHT;
    } else if ( e.from_down ) {
        return LEFT;
    }

    return -1;
}

Border ( *start_border_solver[ 2 ] )( Map *, int, int ) = {
    rhand_start_border,
    lhand_start_border,
};

Border start_border( Map *map, int r, int c, int leftright ) {
    Border border;
    if ( leftright == 0 ) {
        border = start_border_solver[ 0 ]( map, r, c );
    } else if ( leftright == 1 ) {
        border = start_border_solver[ 1 ]( map, r, c );
    } else {
        loginfo( "invalid leftright value %i", leftright );
        return -1;
    }

    if ( (int)border == -1 ) {
        loginfo( "invalid starting point %ix%i", r, c );
    }

    return border;
}
