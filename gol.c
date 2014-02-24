#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <curses.h>

#define VERSION "0.1"

uint8_t **world;
uint16_t size_x, size_y;
uint16_t rules = 0x80c;

void bigbang(void)
{
	uint16_t i, j;

	srand(time(NULL));

	initscr();
	noecho();
	curs_set(0);

	size_x = COLS;
	size_y = LINES;

	if(has_colors()) {
		start_color();
		use_default_colors();
		init_pair(1, rand() % 6 + 1, -1);
		bkgd(COLOR_PAIR(1));
	}

	world = malloc(size_y * sizeof(uint8_t *));
	for(i = 0; i < size_y; i++) {
		world[i] = malloc(2 * size_x * sizeof(uint8_t));
		for(j = 0; j < size_x; j++) {
			world[i][j] = rand() % 2;
		}
	}
}

void apocalypse(void)
{
	uint16_t i;

	for(i = 0; i < size_y; i++)
		free(world[i]);
	free(world);

	endwin();
}

uint8_t neighbours(uint16_t x, uint16_t y)
{
	uint8_t n = -world[x][y];
	int16_t i, j;

	for(i = x - 1; i <= x + 1; i++)
		for(j = y - 1; j <= y + 1; j++)
			if(i >= 0 && i < size_y && j >= 0 && j < size_x)
				n += world[i][j];
	return n;
}

void step(void)
{
	uint16_t i, j;

	for(i = 0; i < size_y; i++)
		for(j = 0; j < size_x; j++)
			if(rules & (1 << (neighbours(i, j) + (world[i][j] ? 0 : 8))))
				world[i][j + size_x] = 1;
			else
				world[i][j + size_x] = 0;

	for(i = 0; i < size_y; i++)
		for(j = size_x; j < 2 * size_x; j++)
			world[i][j - size_x] = world[i][j];
}

void draw(void)
{
	uint16_t i, j;

	for(i = 0; i < size_y; i++)
		for(j = 0; j < size_x; j++)
			mvaddch(i, j, world[i][j] ? ' ' | A_REVERSE : ' ');
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
