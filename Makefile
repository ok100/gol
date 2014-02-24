all: gol

gol: gol.c
	gcc -Wall -Wextra -Os -lcurses -o gol gol.c

clean:
	rm gol
