#include "map.h"

void log_result( int r, int c, Border expected, Border got ) {
    if ( expected == got ) {
        printf( "SUCCESS: %ix%i expected %i, got: %i\n", r, c, expected, got );
    } else {
        printf( "FAIL   : %ix%i expected %i, got: %i\n", r, c, expected, got );
    }
}

void run_case( Map *m, int r, int c, int leftright, Border expected ) {
    Border got = start_border( m, r, c, leftright );
    log_result( 2, 1, expected, got );
}

int main() {
    Map *m = load_map( stdin );
    if ( m == NULL ) {
        fprintf( stderr, "Invalid Map\n" );
        return 1;
    }

    Border b;

    run_case( m, 1, 1, RIGHT_HAND, RIGHT );
    run_case( m, 2, 1, RIGHT_HAND, DOWN );
    run_case( m, 3, 1, RIGHT_HAND, RIGHT );
    run_case( m, 1, 2, RIGHT_HAND, LEFT );
    run_case( m, m->rows, 2, RIGHT_HAND, RIGHT );

    run_case( m, 1, 1, LEFT_HAND, UP );
    run_case( m, 2, 1, LEFT_HAND, RIGHT );
    run_case( m, 3, 1, LEFT_HAND, UP );

    return 0;
}
