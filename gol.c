#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <curses.h>

#define VERSION "0.1"

typedef struct cell {
	bool is_alive;
	uint8_t age;
} Cell;

Cell **world;
size_t size_x, size_y;
uint16_t rules = 0x80c;

void bigbang(void)
{
	size_t i, j;

	srand(time(NULL));

	initscr();
	noecho();
	curs_set(0);

	size_x = COLS;
	size_y = LINES;

	if(has_colors()) {
		start_color();
		use_default_colors();
		init_pair(1, -1, COLOR_RED);
		init_pair(2, -1, COLOR_YELLOW);
		init_pair(3, -1, COLOR_GREEN);
		init_pair(4, -1, COLOR_CYAN);
		init_pair(5, -1, COLOR_BLUE);
	}

	world = malloc(size_y * sizeof(Cell *));
	for(i = 0; i < size_y; i++) {
		world[i] = malloc(2 * size_x * sizeof(Cell));
		for(j = 0; j < size_x; j++) {
			world[i][j].is_alive = rand() % 2;
			world[i][j].age = 1;
		}
	}
}

void apocalypse(void)
{
	size_t i;

	for(i = 0; i < size_y; i++)
		free(world[i]);
	free(world);

	endwin();
}

uint8_t neighbours(size_t x, size_t y)
{
	uint8_t n = -world[x][y].is_alive;
	int16_t i, j;

	for(i = (int)x - 1; i <= (int)x + 1; i++)
		for(j = (int)y - 1; j <= (int)y + 1; j++)
			if(i >= 0 && j >= 0 && i < (int)size_y && j < (int)size_x)
				n += world[i][j].is_alive;
	return n;
}

void step(void)
{
	size_t i, j;

	for(i = 0; i < size_y; i++)
		for(j = 0; j < size_x; j++)
			if(rules & (1 << (neighbours(i, j) + (world[i][j].is_alive ? 0 : 8)))) {
				if(!world[i][j + size_x].is_alive)
					world[i][j + size_x].age = 0;
				else if(world[i][j + size_x].age < 40)
					world[i][j + size_x].age++;
				world[i][j + size_x].is_alive = true;
			} else
				world[i][j + size_x].is_alive = false;

	for(i = 0; i < size_y; i++)
		for(j = size_x; j < 2 * size_x; j++) {
			world[i][j - size_x].is_alive = world[i][j].is_alive;
			world[i][j - size_x].age = world[i][j].age;
		}
}

void draw(void)
{
	size_t i, j;

	for(i = 0; i < size_y; i++)
		for(j = 0; j < size_x; j++) {
			if(has_colors()) {
				if(!world[i][j].is_alive)
					attrset(COLOR_PAIR(0));
				else if(world[i][j].age < 10)
					attrset(COLOR_PAIR(1));
				else if(world[i][j].age < 20)
					attrset(COLOR_PAIR(2));
				else if(world[i][j].age < 30)
					attrset(COLOR_PAIR(3));
				else if(world[i][j].age < 40)
					attrset(COLOR_PAIR(4));
				else
					attrset(COLOR_PAIR(5));
			}
			mvaddch(i, j, ' ');
		}
	refresh();
}

void parse_rules(char *str)
{
	size_t i;
	uint8_t x = 0;

	rules = 0;

	for(i = 0; i < strlen(str); i++)
		str[i] == '/' ? x = 1 : (rules |= 1 << (str[i] - '0' + (x ? 8 : 0)));
}

int main(int argc, char *argv[])
{
	int opt;
	float speed = 0.1;

	while((opt = getopt(argc, argv, "s:r:hv")) != -1) {
		switch(opt) {
		case 's':
			speed = atof(optarg);
			break;
		case 'r':
			parse_rules(optarg);
			break;
		case 'h':
			printf("Usage: %s [-h|-v] [-s SPEED] [-r RULES]\n", argv[0]);
			return 0;
		case 'v':
			puts(VERSION);
			return 0;
		default:
			return 1;
		}
	}

	bigbang();

	while(1) {
		draw();
		step();
		usleep(speed * 1000000);
	}

	apocalypse();

	return 0;
}
