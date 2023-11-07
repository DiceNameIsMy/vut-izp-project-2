#include "map.h"

int main() {
    Map m;
    int r = construct_map( &m, stdin );
    if ( r != 0 ) {
        fprintf( stderr, "Invalid Map\n" );
        return 1;
    }
    return 0;
}
