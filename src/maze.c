#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

typedef struct position {
    int row;
    int column;
} Position;

static const char HELP_TEXT[] = ""
                                "Usage: ./maze [option] file...\n"
                                "Option:\n"
                                "  --help\n"
                                "  --test\n"
                                "  --rpath\n"
                                "  --lpath\n"
                                "  --shortest\n";
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

Map *load_maze( char *filename ) {
    FILE *file = fopen( filename, "r" );
    if ( file == NULL )
        return NULL;

    Map *map = load_map( file );

    fclose( file );

    return map;
}

int try_test_maze( char *option, char *filename ) {
    bool has_test_option = strcmp( option, "--test" ) == 0;
    if ( !has_test_option ) {
        fprintf( stderr, INVALID_ARGS_ERROR, option );
        return 1;
    }

    Map *map = load_maze( filename );

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

int get_starting_position( char *row, char *column, Position *position ) {
    char *p_row;
    int start_row = strtol( row, &p_row, 10 );
    if ( *p_row != '\0' ) {
        fprintf( stderr, INVALID_ARGS_ERROR, row );
        return 1;
    }

    char *p_column;
    int start_column = strtol( column, &p_column, 10 );
    if ( *p_column != '\0' ) {
        fprintf( stderr, INVALID_ARGS_ERROR, column );
        return 1;
    }

    position->row = start_row;
    position->column = start_column;
    return 0;
}

int try_solve_maze( char *option, char *row, char *column, char *filename ) {
    Strategy strategy = get_strategy( option );
    if ( (int)strategy == -1 ) {
        fprintf( stderr, UNKNOWN_STRATEGY_ERROR, option );
        return 1;
    } else if ( strategy == SHORTEST ) {
        fprintf( stderr, "--shortest is not implemented\n" );
        return 1;
    }

    Position start_at;
    int r = get_starting_position( row, column, &start_at );
    if ( r != 0 ) {
        return 1;
    }

    Map *map = load_maze( filename );
    if ( map == NULL ) {
        return 1;
    }
    solve_maze( map, start_at.row, start_at.column, strategy );

    destruct_map( map );
    return 0;
}
