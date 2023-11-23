#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

/*

QUESTIONS:

Should we validate that maze has any exits?
Should we validate that entering cell has an entrance?

*/

static const char HELP_TEXT[] =
    ""
    "Usage: ./maze [options] file...\n"
    "Options:\n"
    "  --help                     Display this information.\n"
    "  --test                     Test if given file is a valid maze.\n"
    "  --rpath <row> <column>     Solve maze using the right hand rule.\n"
    "  --lpath <row> <column>     Solve maze using the left hand rule.\n"
    "  --shortest <row> <column>  Solve maze by finding the shortest path to "
    "the exit.\n";
static const char INVALID_ARGS_ERROR[] =
    "Invalid argument `%s`. Try `maze --help` for more information.\n";
static const char INVALID_ARGS_AMOUNT_ERROR[] =
    "Invalid amount of arguments. Try `maze --help` for more information.\n";
static const char UNKNOWN_STRATEGY_ERROR[] =
    "Unknown strategy `%s`. Try `maze --help` for more information.\n";

bool has_help_flag( int argc, char *argv[] );

int try_test_maze( char *option, char *filename );

int try_solve_maze( char *option, char *row, char *column, char *filename );

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

int set_starting_position( char *str_row, char *str_column,
                           Position *position ) {
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

    position->row = row;
    position->column = column;
    return 0;
}

void print_location( int r, int c ) { printf( "%i,%i\n", r, c ); }

int try_solve_maze( char *option, char *row, char *column, char *filename ) {
    Strategy strategy = get_strategy( option );
    if ( (int)strategy == -1 ) {
        fprintf( stderr, UNKNOWN_STRATEGY_ERROR, option );
        return 1;
    }

    Position start_at;
    if ( set_starting_position( row, column, &start_at ) == -1 ) {
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
    solve_maze( map, start_at.row, start_at.column, strategy, print_location );

    destruct_map( map );
    return 0;
}
