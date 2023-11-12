main:
	gcc -std=c11 -Wall -Wextra -Werror -g src/maze.c src/map.c -o bin/maze
test:
	gcc -std=c11 -Wall -Wextra -Werror src/test.c src/map.c -o bin/test
