#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

#define UNINITIALIZED = -1

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

void test_maze( char *filename );

bool has_help_flag( int argc, char *argv[] );

int main( int argc, char *argv[] ) {
    bool show_help = ( argc == 1 ) || has_help_flag( argc, argv );
    if ( show_help ) {
        printf( HELP_TEXT );
        return 0;
    }

    if ( argc == 3 ) {
        char *flag = argv[ 1 ];
        char *maze_filename = argv[ 2 ];

        bool has_test_flag = strcmp( flag, "--test" ) == 0;
        if ( !has_test_flag ) {
            fprintf( stderr, INVALID_ARGS_ERROR, flag );
            return 1;
        }

        test_maze( maze_filename );
        return 0;

    } else if ( argc == 5 ) {
        char *flag = argv[ 1 ];

        char *row_ptr;
        // int start_row = strtol( argv[ 2 ], &row_ptr, 10 );
        strtol( argv[ 2 ], &row_ptr, 10 );
        if ( *row_ptr != '\0' ) {
            fprintf( stderr, INVALID_ARGS_ERROR, argv[ 2 ] );
            return 1;
        }

        char *column_ptr;
        // int start_column = strtol( argv[ 3 ], &column_ptr, 10 );
        strtol( argv[ 3 ], &column_ptr, 10 );
        if ( *column_ptr != '\0' ) {
            fprintf( stderr, INVALID_ARGS_ERROR, argv[ 3 ] );
            return 1;
        }

        // char *maze_filename = argv[ 4 ];

        if ( strcmp( flag, "--rpath" ) == 0 ) {
            fprintf( stderr, "--rpath is not implemented\n" );
            return 1;
        } else if ( strcmp( flag, "--lpath" ) == 0 ) {
            fprintf( stderr, "--lpath is not implemented\n" );
            return 1;
        } else if ( strcmp( flag, "--shortest" ) == 0 ) {
            fprintf( stderr, "--shortest is not implemented\n" );
            return 1;
        } else {
            fprintf( stderr, UNKNOWN_STRATEGY_ERROR, flag );
            return 1;
        }
    }

    fprintf( stderr, INVALID_ARGS_AMOUNT_ERROR );
    return 1;
}

void test_maze( char *filename ) {
    FILE *maze_file = fopen( filename, "r" );
    if ( maze_file == NULL ) {
        printf( "Invalid\n" );
        return;
    }

    Map map;
    int r = construct_map( &map, maze_file );
    if ( r != 0 ) {
        printf( "Invalid\n" );
        return;
    }

    fclose( maze_file );
    destruct_map( &map );

    printf( "Valid\n" );
}

bool has_help_flag( int argc, char *argv[] ) {
    for ( int i = 0; i < argc; i++ ) {
        if ( strcmp( argv[ i ], "--help" ) == 0 ) {
            return true;
        }
    }
    return false;
}
