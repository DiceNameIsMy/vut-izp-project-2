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

// For Dijkstra's algorithm
#define INFINITY -1

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

/*

DEFINITONS REQUIRED TO BE DEFINED
---------------------------------------------------------------------

*/

/* Represents a map of the maze.

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

    map->cells = malloc( sizeof( unsigned char ) * map->rows * map->cols );
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
    if ( r < 1 || r > map->rows || c < 1 || c > map->cols ) {
        return -1;
    }

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

MAZE SOLVING SECTIONS
---------------------------------------------------------------------

*/

/*
Function to invoke on each step taken to get out of the maze
*/
typedef void ( *on_step_func_t )( int r, int c );

void solve_leftright( Map *map, int r, int c, int leftright,
                      on_step_func_t on_step_func ) {
    Border direction = start_border( map, r, c, leftright );
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
        direction = resolve_direction( map, r, c, leftright, came_from );
        steps++;
    }
}
/*

SHORTEST STRATEGY ALGORITHM
---------------------------------------------------------------------

*/

typedef struct {
    int row;
    int column;
} Position;

typedef struct {
    int amount;
    Position *exits;
} AllExits;

AllExits *init_all_exits() {
    AllExits *all_exits = malloc( sizeof( AllExits ) );
    if ( all_exits == NULL )
        return NULL;

    all_exits->amount = 0;
    all_exits->exits = NULL;
    return all_exits;
}

int add_exit( AllExits *exits, int r, int c ) {
    exits->amount++;
    if ( exits->amount == 0 ) {
        exits->exits = malloc( sizeof( Position ) );
    } else {
        exits->exits =
            realloc( exits->exits, sizeof( Position ) * exits->amount );
    }

    if ( exits->exits == NULL )
        return -1;

    Position p = { .row = r, .column = c };
    exits->exits[ exits->amount - 1 ] = p;

    return 0;
}

void destruct_all_exits( AllExits *exits ) {
    free( exits->exits );
    free( exits );
}

/*
Load all cell locations that contain an open passage out of the maze

Might find 2 positions with the same coordinates when there are 2 open passages
out of the maze
*/
int load_all_exits( Map *map, int r, int c, AllExits *exits ) {
    for ( int row = 1; row <= map->rows; row++ ) {
        // for each leftmost cell
        if ( row == r && c == 1 )
            continue;

        bool has_exit_on_left = !has_left_border( get_cell( map, row, 1 ) );
        if ( has_exit_on_left ) {
            if ( add_exit( exits, row, 1 ) == -1 ) {
                destruct_all_exits( exits );
                return -1;
            }
        }

        // for each rightmost cell
        if ( row == r && c == map->cols )
            continue;

        bool has_exit_on_right =
            !has_right_border( get_cell( map, row, map->cols ) );
        if ( has_exit_on_right ) {
            if ( add_exit( exits, row, map->cols ) == -1 ) {
                destruct_all_exits( exits );
                return -1;
            }
        }
    }

    for ( int col = 1; col <= map->cols; col++ ) {
        // for each uppermost cell
        if ( col == c && r == 1 )
            continue;

        bool has_exit_above = has_passage_above( 1, col ) &&
                              !has_updown_border( get_cell( map, 1, col ) );
        if ( has_exit_above ) {
            if ( add_exit( exits, 1, col ) == -1 ) {
                destruct_all_exits( exits );
                return -1;
            }
        }

        // for each cell on the bottom
        if ( col == c && r == map->rows )
            continue;

        bool has_exit_below =
            !has_passage_above( map->rows, col ) &&
            !has_updown_border( get_cell( map, map->rows, col ) );
        if ( has_exit_below ) {
            if ( add_exit( exits, map->rows, col ) == -1 ) {
                destruct_all_exits( exits );
                return -1;
            }
        }
    }

    return 0;
}

typedef struct {
    bool processed;
    int distance;
} Vertex;

void init_distances( Vertex *distances, Map *map, Position start ) {
    int amount_of_vertices = map->rows * map->cols;
    for ( int i = 0; i < amount_of_vertices; i++ ) {
        Vertex v = { .processed = false, .distance = INFINITY };
        distances[ i ] = v;
    }

    int start_idx = get_cell_idx( map, start.row, start.column );
    distances[ start_idx ].distance = 0;
}

Position get_lowest_vertex_position( Map *map, Vertex *distances ) {
    Position p = { .row = -1, .column = -1 };
    Vertex *v = NULL;

    for ( int r = 1; r <= map->rows; r++ ) {
        for ( int c = 1; c <= map->cols; c++ ) {
            int next_idx = get_cell_idx( map, r, c );
            Vertex *next_vertex = &distances[ next_idx ];
            if ( next_vertex->processed || next_vertex->distance == INFINITY )
                continue;

            if ( v == NULL || v->distance == INFINITY ||
                 v->distance > next_vertex->distance ) {
                p.row = r;
                p.column = c;
                v = next_vertex;
            }
        }
    }

    return p;
}

void process_edge( Map *map, Vertex *distances, Position from,
                   Border direction ) {
    loginfo( "processing a vertex %ix%i moving %s", from.row, from.column,
             border_str[ direction ] );
    if ( direction == UP && !has_passage_above( from.row, from.column ) ) {
        return;
    }
    if ( direction == DOWN && has_passage_above( from.row, from.column ) ) {
        return;
    }

    if ( isborder( map, from.row, from.column, direction ) ) {
        loginfo( "can not move %s because of a border",
                 border_str[ direction ] );
        return;
    }

    if ( moves_out_of_maze( map, from.row, from.column, direction ) ) {
        loginfo( "can not move %s because of moving out if the maze",
                 border_str[ direction ] );
        return;
    }

    Vertex *v_from = &distances[ get_cell_idx( map, from.row, from.column ) ];

    Position pos_to = { .row = move_r( from.row, direction ),
                        .column = move_c( from.column, direction ) };
    Vertex *v_to = &distances[ get_cell_idx( map, pos_to.row, pos_to.column ) ];

    bool is_unset = v_to->distance == INFINITY;
    bool new_path_is_shorter = v_to->distance > ( v_from->distance + 1 );

    if ( is_unset || new_path_is_shorter ) {
        v_to->distance = v_from->distance + 1;
        loginfo( "set %ix%i to a distance of %i", pos_to.row, pos_to.column,
                 v_to->distance );
    }
}

int find_shortest_path( Map *map, Position start, Position end,
                        Vertex *distances ) {
    while ( true ) {
        Position p = get_lowest_vertex_position( map, distances );

        if ( p.row == -1 && p.column == -1 ) {
            loginfo( "failed to find a way to exit a maze by starting at %ix%i "
                     "(not sure)",
                     start.row, start.column );
            return -1;
        }
        if ( p.row == end.row && p.column == end.column ) {
            loginfo( "vertex with lowest value at %ix%i is the end", p.row,
                     p.column );
            return 0;
        }

        Vertex *v = &distances[ get_cell_idx( map, p.row, p.column ) ];

        loginfo( "next vertex: %ix%i distance from origin: %i", p.row, p.column,
                 v->distance );

        process_edge( map, distances, p, LEFT );
        process_edge( map, distances, p, RIGHT );
        process_edge( map, distances, p, UP );
        process_edge( map, distances, p, DOWN );

        v->processed = true;
        loginfo( "vertex %ix%i is processed", p.row, p.column );
    }
}

void solve_shortest( Map *map, int r, int c ) {
    Border entered_from = entered_maze_from( map, r, c );
    if ( (int)entered_from == -1 ) {
        // TODO: log the problem
        return;
    }

    AllExits *exits = init_all_exits();
    if ( load_all_exits( map, r, c, exits ) == -1 ) {
        // TODO: log teh problem
        destruct_all_exits( exits );
    }

    if ( exits->amount == 0 ) {
        // TODO: log the problem
        return;
    }

    Position start = { .row = r, .column = c };

    Position *shortest_exit = NULL;
    Vertex *shortest_vertex = NULL;
    Vertex *shortest_distances = NULL;

    for ( int i = 0; i < exits->amount; i++ ) {
        Position *end = &exits->exits[ i ];
        loginfo( "found cell with exit at %ix%i", end->row, end->column );

        int amount_of_vertices = map->rows * map->cols;
        Vertex *distances = malloc( sizeof( Vertex ) * amount_of_vertices );
        if ( distances == NULL ) {
            // TODO: log the problem
            return;
        }

        init_distances( distances, map, start );

        find_shortest_path( map, start, *end, distances );

        printf( "exit is located at %ix%i\n", end->row, end->column );
        for ( int r = 1; r <= map->rows; r++ ) {
            for ( int c = 1; c <= map->cols; c++ ) {
                int cell_distance =
                    distances[ get_cell_idx( map, r, c ) ].distance;
                printf( "[%02i]", cell_distance );
            }
            printf( "\n" );
        }

        if ( shortest_exit == NULL ) {
            shortest_exit = end;
            shortest_distances = distances;
            int idx =
                get_cell_idx( map, shortest_exit->row, shortest_exit->column );
            shortest_vertex = &shortest_distances[ idx ];
            continue;
        }

        Vertex *end_vertex =
            &distances[ get_cell_idx( map, end->row, end->column ) ];
        if ( end_vertex->distance < shortest_vertex->distance ) {
            free( shortest_distances );

            shortest_exit = end;
            shortest_distances = distances;
            int idx =
                get_cell_idx( map, shortest_exit->row, shortest_exit->column );
            shortest_vertex = &shortest_distances[ idx ];
            continue;
        }

        free( distances );
    }

    // TODO: find the shortest path with reversed points to be able to find the
    // shortest path & act on it

    // act on the shortest path

    printf( "shortest exit is located at %ix%i\n", shortest_exit->row,
            shortest_exit->column );
    for ( int r = 1; r <= map->rows; r++ ) {
        for ( int c = 1; c <= map->cols; c++ ) {
            int cell_distance =
                shortest_distances[ get_cell_idx( map, r, c ) ].distance;
            printf( "[%02i]", cell_distance );
        }
        printf( "\n" );
    }

    destruct_all_exits( exits );
    free( shortest_distances );
}

/*

MAZE SOLVER ENTRYPOINT
---------------------------------------------------------------------

*/

void solve_maze( Map *map, int r, int c, Strategy strategy,
                 on_step_func_t on_step_func ) {
    if ( strategy == SHORTEST ) {
        solve_shortest( map, r, c );
        return;
    }

    solve_leftright( map, r, c, strategy, on_step_func );
}

/*

COMMAND LINE INTERFACE (CLI)
---------------------------------------------------------------------

*/

bool has_help_flag( int argc, char *argv[] );

int try_test_maze( char *option, char *filename );

int try_solve_maze( char *option, char *row, char *column, char *filename );

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

int to_positive_int( char *str ) {
    char *p_str;
    int val = strtol( str, &p_str, 10 );
    return *p_str == '\0' ? val : -1;
}

int set_starting_position( char *str_row, char *str_column, int *p_row,
                           int *p_col ) {
    int row = to_positive_int( str_row );
    if ( row == -1 ) {
        fprintf( stderr, INVALID_ARGS_ERROR, str_row );
        return -1;
    }
    int column = to_positive_int( str_column );
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
