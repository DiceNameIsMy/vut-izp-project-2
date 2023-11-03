#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

static const char HELP_TEXT[] = ""
                                "Usage: ./maze [option] file...\n"
                                "Option:\n"
                                "  --help\n"
                                "  --test\n"
                                "  --rpath\n"
                                "  --lpath\n"
                                "  --shortest\n";
static const char INVALID_ARGS_ERROR[] =
    "Invalid amount of arguments. See --help for more information.\n";

int main(int argc, char *argv[]) {
    char *flag = NULL;
    char *maze_filename = NULL;

    for (int i = 1; i < argc; i++) {
        char *argument = argv[i];
        bool is_flag = strncmp(argument, "--", 2) == 0;

        if (is_flag) {
            if (flag != NULL) {
                fprintf(stderr, INVALID_ARGS_ERROR);
                return 1;
            }
            flag = argument;
        } else {
            if (maze_filename != NULL) {
                fprintf(stderr, INVALID_ARGS_ERROR);
                return 1;
            }
            maze_filename = argument;
        }
    }

    bool no_args = (flag == NULL && maze_filename == NULL);
    if (no_args || (strcmp(flag, "--help") == 0)) {
        printf(HELP_TEXT);
        return 0;
    }

    if (strcmp(flag, "--test") == 0) {
        FILE *maze_file = fopen(maze_filename, "r");
        if (maze_file == NULL) {
            printf("Invalid\n");
            return 0;
        }

        Map map;
        int r = construct_map(&map, maze_file);
        if (r != 0) {
            printf("Invalid\n");
            return 0;
        }

        fclose(maze_file);
        destruct_map(&map);

        printf("Valid\n");
    }

    return 0;
}
