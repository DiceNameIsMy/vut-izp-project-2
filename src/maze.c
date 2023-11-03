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
static const char INVALID_ARGS_AMOUNT_ERROR[] =
    "Invalid amount of arguments. See --help for more information.\n";
static const char UNKNOWN_STATEGY_ERROR[] =
    "Unknown strategy `%s`. See --help for more information.\n";

void test_maze(char *filename) {
    FILE *maze_file = fopen(filename, "r");
    if (maze_file == NULL) {
        printf("Invalid\n");
        return;
    }

    Map map;
    int r = construct_map(&map, maze_file);
    if (r != 0) {
        printf("Invalid\n");
        return;
    }

    fclose(maze_file);
    destruct_map(&map);

    printf("Valid\n");
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf(HELP_TEXT);
        return 0;
    }
    if (argc > 3) {
        fprintf(stderr, INVALID_ARGS_AMOUNT_ERROR);
        return 1;
    }

    char *flag = NULL;
    char *maze_filename = NULL;

    for (int i = 1; i < argc; i++) {
        char *argument = argv[i];

        if (strcmp(argument, "--help") == 0) {
            printf(HELP_TEXT);
            return 0;
        } else if (strncmp(argument, "--", 2) == 0) {
            if (flag != NULL) {
                fprintf(stderr, INVALID_ARGS_AMOUNT_ERROR);
                return 1;
            }
            flag = argument;
        } else {
            if (maze_filename != NULL) {
                fprintf(stderr, INVALID_ARGS_AMOUNT_ERROR);
                return 1;
            }
            maze_filename = argument;
        }
    }

    if (strcmp(flag, "--test") == 0) {
        test_maze(maze_filename);
    } else if (strcmp(flag, "--rpath") == 0) {
        fprintf(stderr, "--rpath is not implemented\n");
        return 1;
    } else if (strcmp(flag, "--lpath") == 0) {
        fprintf(stderr, "--lpath is not implemented\n");
        return 1;
    } else if (strcmp(flag, "--shortest") == 0) {
        fprintf(stderr, "--shortest is not implemented\n");
        return 1;
    } else {
        fprintf(stderr, UNKNOWN_STATEGY_ERROR, flag);
    }

    return 0;
}
