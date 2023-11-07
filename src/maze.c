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

void test_maze( char *filename );

int get_starting_position( char *row, char *column, Position *position );

int main( int argc, char *argv[] ) {
    bool show_help = ( argc == 1 ) || has_help_flag( argc, argv );
    if ( show_help ) {
        printf( HELP_TEXT );
        return 0;
    }

    if ( argc == 3 ) {
        char *flag = argv[ 1 ];
        char *maze_filename = argv[ 2 ];

        bool has_test_option = strcmp( flag, "--test" ) == 0;
        if ( !has_test_option ) {
            fprintf( stderr, INVALID_ARGS_ERROR, flag );
            return 1;
        }

        test_maze( maze_filename );
        return 0;

    } else if ( argc == 5 ) {
        char *solving_strategy = argv[ 1 ];

        Position start_at;
        int r = get_starting_position( argv[ 2 ], argv[ 3 ], &start_at );
        if ( r != 0 ) {
            return 1;
        }

        // char *maze_filename = argv[ 4 ];

        if ( strcmp( solving_strategy, "--rpath" ) == 0 ) {
            fprintf( stderr, "--rpath is not implemented\n" );
            return 1;
        } else if ( strcmp( solving_strategy, "--lpath" ) == 0 ) {
            fprintf( stderr, "--lpath is not implemented\n" );
            return 1;
        } else if ( strcmp( solving_strategy, "--shortest" ) == 0 ) {
            fprintf( stderr, "--shortest is not implemented\n" );
            return 1;
        } else {
            fprintf( stderr, UNKNOWN_STRATEGY_ERROR, solving_strategy );
            return 1;
        }
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

void test_maze( char *filename ) {
    FILE *maze_file = fopen( filename, "r" );
    if ( maze_file == NULL ) {
        printf( "Invalid\n" );
        return;
    }

    Map *map = load_map( maze_file );
    fclose( maze_file );

    if ( map == NULL ) {
        printf( "Invalid\n" );
        return;
    }
    printf( "Valid\n" );
    destruct_map( map );
}

int get_starting_position( char *row, char *column, Position *position ) {
    char *row_ptr;
    int start_row = strtol( row, &row_ptr, 10 );
    if ( *row_ptr != '\0' ) {
        fprintf( stderr, INVALID_ARGS_ERROR, row );
        return 1;
    }

    char *column_ptr;
    int start_column = strtol( column, &column_ptr, 10 );
    if ( *column_ptr != '\0' ) {
        fprintf( stderr, INVALID_ARGS_ERROR, column );
        return 1;
    }

    position->row = start_row;
    position->column = start_column;
    return 0;
}
