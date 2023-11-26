/*

QUESTIONS:

Should we validate that maze has any exits?
Should we validate that entering cell has an entrance?

*/

// #define NDEBUG

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

typedef enum border { RIGHT, LEFT, UP, DOWN, BORDER_COUNT } Border;

/*
For displaying the border in log message as a word instead of a number
*/
char *border_str[ BORDER_COUNT ] = {
    [RIGHT] = "RIGHT",
    [LEFT] = "LEFT",
    [UP] = "UP",
    [DOWN] = "DOWN",
};

/* ## Represents a map of the maze.

cells - each char represents a cell with 3 passages in the maze.

Cell has 3 passages that can have borders that prevent moving through them. It
is represented by an 8 bit integer. 3 rightmost bits are used. Value of 0
specifies the absence of a border on the passage, and 1 its presence.

First bit (0b001) -> border on the left
Second bit (0b010) -> border on the right
Third bit (0b10) -> border is at the passage above or below. It depends on the
sum of the coordinates:
    - odd (1 + 2): passage is above
    - even (1 + 1): passage is below
*/
typedef struct map {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

/*
Returns whether there is a border at the given passage. The parameter with name
`int border` does not make much sense in this implementation. Unfortunately,
there is a specific requirement to have it defined like this.

`int border` represents a direction where to perform the check.
*/
bool isborder( Map *map, int r, int c, int border );

/*
Get the border to start crossing on the first step.

`int leftright` represents the strategy of how to "solve" the maze. See enum
`Strategy` for more context.

Output: `int` represents a Border where to move on the next step.
*/
int start_border( Map *map, int r, int c, int leftright );

/*

HELPER DEFINITIONS
---------------------------------------------------------------------

*/

int get_cell_idx( Map *map, int r, int c ) {
    int row_skip_cells = ( r - 1 ) * map->cols;
    int col_skip_cells = c - 1;
    return row_skip_cells + col_skip_cells;
}

int get_cell( Map *map, int r, int c ) {
    int idx = get_cell_idx( map, r, c );
    int val = map->cells[ idx ];
    return val;
}

bool has_left_border( int cell ) { return ( cell & 0b001 ) == 0b001; }
bool has_right_border( int cell ) { return ( cell & 0b010 ) == 0b010; }
bool has_updown_border( int cell ) { return ( cell & 0b100 ) == 0b100; }

bool out_of_maze( Map *m, int r, int c ) {
    return ( r < 1 || r > m->rows || c < 1 || c > m->cols );
}

int row_incr[ BORDER_COUNT ] = {
    [RIGHT] = 0,
    [LEFT] = 0,
    [UP] = -1,
    [DOWN] = 1,
};
int col_incr[ BORDER_COUNT ] = {
    [RIGHT] = 1,
    [LEFT] = -1,
    [UP] = 0,
    [DOWN] = 0,
};

int move_r( int r, Border direction ) { return r + row_incr[ direction ]; }

int move_c( int c, Border direction ) { return c + col_incr[ direction ]; }

bool moves_out_of_maze( Map *m, int r, int c, Border direction ) {
    int moved_r = move_r( r, direction );
    int moved_c = move_c( c, direction );
    return out_of_maze( m, moved_r, moved_c );
}

bool has_passage_above( int r, int c ) { return ( ( ( r + c ) & 1 ) == 0 ); }

Border reverse_direction[ BORDER_COUNT ] = {
    [RIGHT] = LEFT, [LEFT] = RIGHT, [UP] = DOWN, [DOWN] = UP };

/*

MAP INITIALIZATION & DESTRUCTION
---------------------------------------------------------------------

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

int to_cell( char ch, int *cell ) {
    bool is_allowed_number = ch >= '0' && ch <= '7';
    if ( !is_allowed_number )
        return -1;

    *cell = ch - '0';
    return 1;
}

int process_char( Map *map, char ch, int *row_ptr, int *col_ptr ) {
    if ( ch == ' ' ) {
        return 0;
    }
    if ( ch == '\n' ) {
        bool all_row_columns_set = ( *col_ptr - 1 ) == map->cols;
        if ( !all_row_columns_set )
            return -1;

        loginfo( "moving to the next line(row) %i->%i", *row_ptr,
                 *row_ptr + 1 );

        ( *row_ptr )++;
        *col_ptr = 1;

        return 0;
    }

    int cell;
    if ( to_cell( ch, &cell ) == -1 )
        return -1;

    int cell_idx = get_cell_idx( map, *row_ptr, *col_ptr );

    loginfo( "adding cell: %i at %ix%i with idx %i", cell, *row_ptr, *col_ptr,
             cell_idx );

    map->cells[ cell_idx ] = cell;
    ( *col_ptr )++;

    return 0;
}

int read_map_cells( Map *map, FILE *file ) {
    int row = 1;
    int column = 1;

    char c;
    while ( ( c = fgetc( file ) ) != EOF ) {
        if ( process_char( map, c, &row, &column ) == -1 )
            return -1;
    }
    // At the end of the file there should be a '\n' that moves the row pointer
    // to the next row. That's why comparing with `map->rows + 1`
    bool all_rows_processed = row == ( map->rows + 1 );
    if ( !all_rows_processed ) {
        return -1;
    }
    return 0;
}

bool check_right_border( Map *map, int cell, int r, int c ) {
    if ( moves_out_of_maze( map, r, c, RIGHT ) )
        return true;

    int right_cell = get_cell( map, r, c + 1 );

    bool mismatch = has_right_border( cell ) ^ has_left_border( right_cell );
    if ( mismatch )
        return false;

    return true;
}

bool check_down_border( Map *map, int cell, int r, int c ) {
    if ( has_passage_above( r, c ) )
        return true;

    if ( moves_out_of_maze( map, r, c, DOWN ) )
        return true;

    int cell_below = get_cell( map, r + 1, c );

    bool mismatch = has_updown_border( cell ) ^ has_updown_border( cell_below );
    if ( mismatch )
        return false;

    return true;
}

bool check_cell_valid( Map *map, int r, int c ) {
    int cell = get_cell( map, r, c );

    bool right_passage_valid = check_right_border( map, cell, r, c );
    bool down_passage_valid = check_down_border( map, cell, r, c );

    return right_passage_valid && down_passage_valid;
}

bool is_map_valid( Map *map ) {
    for ( int r = 1; r <= map->rows; r++ ) {
        for ( int c = 1; c <= map->cols; c++ ) {
            bool cell_valid = check_cell_valid( map, r, c );
            if ( !cell_valid )
                return false;
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
    if ( map == NULL )
        return NULL;

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
---------------------------------------------------------------------

*/

bool isborder( Map *map, int r, int c, int border ) {
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

typedef enum strategy { RIGHT_HAND = 0, LEFT_HAND = 1, SHORTEST = 2 } Strategy;

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

/*
Resolve where to move next. If there is a border on the passage, try the next
best option.
*/
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

int start_border( Map *map, int r, int c, int leftright ) {
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

Command Line Interface (CLI)
---------------------------------------------------------------------

*/

bool has_help_flag( int argc, char *argv[] );

int try_test_maze( char *option, char *filename );

int try_solve_maze( char *option, char *row, char *column, char *filename );

/*
Function to invoke on each step taken to get out of the maze
*/
typedef void ( *on_step_func_t )( int r, int c );

void solve_maze( Map *map, int r, int c, Strategy strategy,
                 on_step_func_t on_step_func );

static const char HELP_TEXT[] =
    ""
    "Usage: ./maze [options] file...\n"
    "Options:\n"
    "  --help                     Display this information.\n"
    "  --test                     Test if given file is a valid maze.\n"
    "  --rpath <row> <column>     Solve maze using the right hand rule.\n"
    "  --lpath <row> <column>     Solve maze using the left hand rule.\n"
    "  --shortest <row> <column>  Solve maze by finding the shortest path "
    "to "
    "the exit.\n";
static const char INVALID_ARGS_ERROR[] =
    "Invalid argument `%s`. Try `maze --help` for more information.\n";
static const char INVALID_ARGS_AMOUNT_ERROR[] =
    "Invalid amount of arguments. Try `maze --help` for more information.\n";
static const char UNKNOWN_STRATEGY_ERROR[] =
    "Unknown strategy `%s`. Try `maze --help` for more information.\n";

int main( int argc, char *argv[] ) {
    bool show_help = has_help_flag( argc, argv ) || ( argc == 1 );
    if ( show_help ) {
        printf( HELP_TEXT );
        return 0;
    }

    if ( argc == 3 ) {
        char *flag = argv[ 1 ];
        char *maze_filename = argv[ 2 ];
        return try_test_maze( flag, maze_filename );
    } else if ( argc == 5 ) {
        char *flag = argv[ 1 ];
        char *row_str = argv[ 2 ];
        char *column_str = argv[ 3 ];
        char *maze_filename = argv[ 4 ];
        return try_solve_maze( flag, row_str, column_str, maze_filename );
    }

    fprintf( stderr, INVALID_ARGS_AMOUNT_ERROR );
    return 1;
}

bool has_help_flag( int argc, char *argv[] ) {
    for ( int i = 0; i < argc; i++ ) {
        if ( strcmp( argv[ i ], "--help" ) == 0 ) {
            return true;
        }
    }
    return false;
}

int try_test_maze( char *option, char *filename ) {
    bool has_test_option = strcmp( option, "--test" ) == 0;
    if ( !has_test_option ) {
        fprintf( stderr, INVALID_ARGS_ERROR, option );
        return 1;
    }

    FILE *file = fopen( filename, "r" );
    if ( file == NULL ) {
        fprintf( stderr, "Failed to read file.\n" );
        return 1;
    }

    Map *map = load_map( file );
    fclose( file );

    if ( map == NULL ) {
        printf( "Invalid\n" );
        return 0;
    }
    printf( "Valid\n" );
    destruct_map( map );

    return 0;
}

Strategy get_strategy( char *option ) {
    if ( strcmp( option, "--rpath" ) == 0 ) {
        return RIGHT_HAND;
    } else if ( strcmp( option, "--lpath" ) == 0 ) {
        return LEFT_HAND;
    } else if ( strcmp( option, "--shortest" ) == 0 ) {
        return SHORTEST;
    }
    return -1;
}

int parse_positive_int( char *str ) {
    char *p_str;
    int val = strtol( str, &p_str, 10 );
    return *p_str == '\0' ? val : -1;
}

int set_starting_position( char *str_row, char *str_column, int *p_row,
                           int *p_col ) {
    int row = parse_positive_int( str_row );
    if ( row == -1 ) {
        fprintf( stderr, INVALID_ARGS_ERROR, str_row );
        return -1;
    }
    int column = parse_positive_int( str_column );
    if ( column == -1 ) {
        fprintf( stderr, INVALID_ARGS_ERROR, str_column );
        return -1;
    }

    *p_row = row;
    *p_col = column;
    return 0;
}

void print_location( int r, int c ) { printf( "%i,%i\n", r, c ); }

int try_solve_maze( char *option, char *row, char *column, char *filename ) {
    Strategy strategy = get_strategy( option );
    if ( (int)strategy == -1 ) {
        fprintf( stderr, UNKNOWN_STRATEGY_ERROR, option );
        return 1;
    }

    int start_row;
    int start_col;
    if ( set_starting_position( row, column, &start_row, &start_col ) == -1 ) {
        return 1;
    }

    FILE *file = fopen( filename, "r" );
    if ( file == NULL ) {
        fprintf( stderr, "Failed to read file.\n" );
        return 1;
    }

    Map *map = load_map( file );
    fclose( file );
    if ( map == NULL ) {
        return 1;
    }
    solve_maze( map, start_row, start_col, strategy, print_location );

    destruct_map( map );
    return 0;
}

void solve_maze( Map *map, int r, int c, Strategy strategy,
                 on_step_func_t on_step_func ) {
    if ( strategy == SHORTEST ) {
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
