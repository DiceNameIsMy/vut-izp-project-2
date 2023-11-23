// #define NDEBUG

#include "map.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#define loginfo( s, ... ) \
    fprintf( stderr, "[INF]" __FILE__ ":%u: " s "\n", __LINE__, __VA_ARGS__ )
#else
#define loginfo( s, ... )
#endif

/*

HELPER DEFINITIONS

*/

char *border_str[ BORDER_COUNT ] = {
    [RIGHT] = "RIGHT",
    [LEFT] = "LEFT",
    [UP] = "UP",
    [DOWN] = "DOWN",
};

int to_cell( char c ) {
    bool is_valid_cell = c >= '0' && c <= '7';
    if ( !is_valid_cell )
        return -1;

    return c - '0';
}

int get_cell_idx( Map *map, int r, int c ) {
    return ( ( r - 1 ) * map->cols ) + c - 1;
}

int get_cell( Map *map, int r, int c ) {
    int idx = get_cell_idx( map, r, c );
    int val = map->cells[ idx ];
    return val;
}

bool out_of_maze( Map *m, int r, int c ) {
    return ( r < 1 || r > m->rows || c < 1 || c > m->cols );
}

/* Mapping of how does row and column change when moving to some direction.
First idx represents Border enum.
Second idx represents row(0) and column(1).
*/
int move_incr[ BORDER_COUNT ][ 2 ] = {
    [RIGHT] = { 0, 1 },
    [LEFT] = { 0, -1 },
    [UP] = { -1, 0 },
    [DOWN] = { 1, 0 },
};

int move_r( int r, Border direction ) {
    return r + move_incr[ direction ][ 0 ];
}

int move_c( int c, Border direction ) {
    return c + move_incr[ direction ][ 1 ];
}

bool moves_out_of_maze( Map *m, int r, int c, Border direction ) {
    int moved_r = move_r( r, direction );
    int moved_c = move_c( c, direction );
    return out_of_maze( m, moved_r, moved_c );
}

bool has_passage_above( int r, int c ) { return ( ( ( r + c ) & 1 ) == 0 ); }

/*

MAP INITIALIZATION & DESTRUCTION

*/

int read_map_size( Map *map, FILE *file ) {
    int rows;
    int columns;

    int r = fscanf( file, "%i %i\n", &rows, &columns );

    bool loaded_both_items = r == 2;

    if ( !loaded_both_items || rows < 1 || columns < 1 )
        return -1;

    map->rows = rows;
    map->cols = columns;

    return 0;
}

typedef enum processCharResult { OK, BAD_MAP, BAD_CELL } ProcessCharResult;

ProcessCharResult process_char( Map *map, char ch, int *row_ptr,
                                int *col_ptr ) {
    if ( ch == ' ' ) {
        return OK;
    } else if ( ch == '\n' ) {
        bool all_columns_set = ( *col_ptr - 1 ) == map->cols;
        if ( !all_columns_set )
            return BAD_MAP;

        loginfo( "moving to the next line(row) %i->%i", *row_ptr,
                 *row_ptr + 1 );

        ( *row_ptr )++;
        *col_ptr = 1;

        return OK;
    }

    int cell = to_cell( ch );
    if ( cell == -1 ) {
        return BAD_CELL;
    }

    int cell_idx = get_cell_idx( map, *row_ptr, *col_ptr );

    loginfo( "adding cell: %i at %ix%i with idx %i", cell, *row_ptr, *col_ptr,
             cell_idx );
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
            return -1;
        } else if ( r == BAD_CELL ) {
            loginfo( "encountered char `%c` which is not a valid number", c );
            return -1;
        }
    }
    return 0;
}

bool has_left_border( int cell ) { return ( cell & 0b001 ) == 0b001; }
bool has_right_border( int cell ) { return ( cell & 0b010 ) == 0b010; }
bool has_updown_border( int cell ) { return ( cell & 0b100 ) == 0b100; }

bool check_right_border( Map *map, int cell, int r, int c ) {
    if ( moves_out_of_maze( map, r, c, RIGHT ) ) {
        loginfo( "went out of bounds at %ix%i", r, c + 1 );
        return true;
    }
    int right_cell = get_cell( map, r, c + 1 );

    loginfo( "comparing cell %i at %ix%i with cell on right %i at %ix%i", cell,
             r, c, right_cell, r, c + 1 );

    if ( has_right_border( cell ) ^ has_left_border( right_cell ) ) {
        loginfo( "map cell at %ix%i with value `%i` is in a mismatch with its "
                 "right cell `%i`",
                 r, c, cell, right_cell );
        return false;
    }
    return true;
}

bool check_down_border( Map *map, int cell, int r, int c ) {
    bool cell_goes_up = has_passage_above( r, c );
    if ( cell_goes_up )
        return true;

    if ( moves_out_of_maze( map, r, c, DOWN ) ) {
        loginfo( "went out of bounds at %ix%i", r + 1, c );
        return true;
    }
    int cell_below = get_cell( map, r + 1, c );

    loginfo( "comparing cell %i at %ix%i with cell below %i at %ix%i", cell, r,
             c, cell_below, r + 1, c );

    if ( has_updown_border( cell ) ^ has_updown_border( cell_below ) ) {
        loginfo( "cell at %ix%i with value `%i` is in a mismatch with a cell "
                 "beneath it with value `%i`",
                 r, c, cell, cell_below );
        return false;
    }

    return true;
}

bool check_cell_valid( Map *map, int r, int c ) {
    int cell = get_cell( map, r, c );

    bool right_border_valid = check_right_border( map, cell, r, c );
    bool down_border_valid = check_down_border( map, cell, r, c );

    return right_border_valid && down_border_valid;
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

void destruct_map( Map *map ) {
    free( map->cells );
    free( map );
}

Map *load_map( FILE *file ) {
    Map *map = malloc( sizeof( Map ) );
    if ( map == NULL ) {
        free( map );
        return NULL;
    }

    if ( read_map_size( map, file ) == -1 ) {
        free( map );
        return NULL;
    }
    loginfo( "map size is %ix%i", map->rows, map->cols );

    map->cells = malloc( map->rows * map->cols );
    if ( map->cells == NULL ) {
        free( map );
        return NULL;
    }

    if ( read_map_cells( map, file ) == -1 ) {
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

/* Resolve to what passage to look at next
- first accessor is a strategy: LEFT_HAND / RIGHT_HAND
- second accessor: Border where from player came
- third accessor: Whether player's current cell has a passage UP
 */
Border next_step_ruleset[ 2 ][ BORDER_COUNT ][ 2 ] = {
    [RIGHT_HAND] = { [RIGHT] = { LEFT, UP },
                     [LEFT] = { DOWN, RIGHT },
                     [UP] = { LEFT, LEFT },
                     [DOWN] = { RIGHT, RIGHT } },
    [LEFT_HAND] = { [RIGHT] = { DOWN, LEFT },
                    [LEFT] = { RIGHT, UP },
                    [UP] = { RIGHT, RIGHT },
                    [DOWN] = { LEFT, LEFT } } };

Border entered_maze_from( Map *map, int r, int c ) {
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

Border resolve_direction( Map *m, int r, int c, Strategy leftright,
                          Border came_from ) {
    bool can_go_up = has_passage_above( r, c );

    Border dir = next_step_ruleset[ leftright ][ came_from ][ can_go_up ];

    if ( isborder( m, r, c, dir ) ) {
        loginfo( "can not go %s", border_str[ dir ] );
        return resolve_direction( m, r, c, leftright, dir );
    }
    return dir;
}

Border reverse_direction[ BORDER_COUNT ] = {
    [RIGHT] = LEFT, [LEFT] = RIGHT, [UP] = DOWN, [DOWN] = UP };

Border start_border( Map *map, int r, int c, int leftright ) {
    if ( leftright == SHORTEST ) {
        loginfo( "invalid leftright value %i", leftright );
        return -1;
    }

    Border entered_from = entered_maze_from( map, r, c );
    if ( (int)entered_from == -1 ) {
        loginfo( "not entering the maze from its borders. Maze size: %ix%i", r,
                 c );
        return -1;
    }
    loginfo( "entered maze from %s", border_str[ entered_from ] );

    return resolve_direction( map, r, c, leftright, entered_from );
}

/*

SHORTEST PATH ALGORITHM

*/

typedef struct path {
    int r;
    int c;
    int depth;
    struct path *next;
} Path;

Path *init_path( int r, int c, int depth ) {
    Path *path = malloc( sizeof( Path ) );
    path->r = r;
    path->c = c;
    path->depth = depth;
    path->next = NULL;
    return path;
}

void destruct_path( Path *p ) { free( p ); }

void destruct_all_paths( Path *p ) {
    if ( p->next != NULL )
        destruct_all_paths( p->next );

    destruct_path( p );
}

void log_weights( Map *m, int *weights ) {
    for ( int i = 1; i <= m->rows; i++ ) {
        for ( int j = 1; j <= m->cols; j++ ) {
            int w = weights[ get_cell_idx( m, i, j ) ];
            fprintf( stderr, "[%i]", w );
        }
        fprintf( stderr, "\n" );
    }
}

bool run_iteration( Map *map, Path *from, int *weights ) {
    bool moves_out_of_maze = out_of_maze( map, from->r, from->c );
    if ( moves_out_of_maze )
        return true;

    int *iteration_depth = &weights[ get_cell_idx( map, from->r, from->c ) ];

    bool reached_faster_in_less_steps =
        ( *iteration_depth != -1 ) && ( from->depth >= *iteration_depth );
    if ( reached_faster_in_less_steps ) {
        return false;
    }

    *iteration_depth = from->depth;

    loginfo( "iterating at %ix%i, with depth %i", from->r, from->c,
             from->depth );

    Path *shortest_path = NULL;

    // For each possible direction
    for ( Border direction = 0; direction < BORDER_COUNT; direction++ ) {
        bool has_border = isborder( map, from->r, from->c, direction );
        if ( has_border )
            continue;

        int next_r = move_r( from->r, direction );
        int next_c = move_c( from->c, direction );
        Path *next = init_path( next_r, next_c, from->depth + 1 );

        int found_exit = run_iteration( map, next, weights );
        if ( !found_exit ) {
            continue;
        }

        // Check if this iteration is the shortest
        if ( shortest_path == NULL ) {
            shortest_path = next;
            continue;
        }

        bool next_path_is_shorter = next->depth < shortest_path->depth;
        if ( next_path_is_shorter ) {
            destruct_all_paths( shortest_path );
            shortest_path = next;
        } else {
            destruct_all_paths( next );
        }
        //
    }

    if ( shortest_path == NULL ) {
        return false;
    } else {
        from->next = shortest_path;
        return true;
    }
}

int *init_weights( Map *map ) {
    int amount_of_cells = 1;
    amount_of_cells *= (int)map->rows;
    amount_of_cells *= (int)map->cols;

    int *weights = malloc( sizeof( int ) * amount_of_cells );
    if ( weights == NULL ) {
        // TODO
    }
    for ( int i = 0; i < amount_of_cells; i++ ) {
        weights[ i ] = -1;
    }

    return weights;
}

void destruct_weights( int *weights ) { free( weights ); }

void solve_shortest( Map *map, int r, int c, on_step_func_t on_step_func ) {
    Path *path = init_path( r, c, 1 );
    int *weights = init_weights( map );

    run_iteration( map, path, weights );

    if ( path->next == NULL ) {
        // there is no exit
        return;
    }

    Path *next = path;
    while ( next != NULL && !out_of_maze( map, next->r, next->c ) ) {
        on_step_func( next->r, next->c );

        Path *prev = next;
        next = next->next;
        destruct_path( prev );
    }
    log_weights( map, weights );

    destruct_weights( weights );
}

/*

NOT GROUPED YET

*/

bool isborder( Map *map, int r, int c, Border border ) {
    int cell = get_cell( map, r, c );

    if ( border == LEFT )
        return has_left_border( cell );
    else if ( border == RIGHT )
        return has_right_border( cell );
    else if ( border == DOWN ) {
        bool has_border = ( cell & 0b100 ) == 0b100;
        return has_passage_above( r, c ) || has_border;
    } else if ( border == UP ) {
        bool has_border = ( cell & 0b100 ) == 0b100;
        return !has_passage_above( r, c ) || has_border;
    }

    loginfo( "invalid border value: %i. this state should be unreachable",
             border );
    return true;
}

void solve_maze( Map *map, int r, int c, Strategy strategy,
                 on_step_func_t on_step_func ) {
    if ( strategy == SHORTEST ) {
        solve_shortest( map, r, c, on_step_func );
        return;
    }

    Border direction = start_border( map, r, c, strategy );
    if ( (int)direction == -1 ) {
        loginfo( "failed to get the starting border with entering points %ix%i",
                 r, c );
        return;
    }
    loginfo( "starting direction is %s", border_str[ direction ] );

    int steps = 0;
    while ( true ) {
        // Act
        on_step_func( r, c );

        // Move
        r = move_r( r, direction );
        c = move_c( c, direction );
        if ( out_of_maze( map, r, c ) ) {
            loginfo( "exit from maze was found in %i steps", steps );
            return;
        }

        Border came_from = reverse_direction[ direction ];
        direction = resolve_direction( map, r, c, strategy, came_from );
        steps++;
    }
}
