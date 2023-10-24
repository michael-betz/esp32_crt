// demo the drawing engine on a PC using SDL2
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include "draw.h"
#include "fast_sin.h"
#include "print.h"

#define DISPLAY_WIDTH 1024
#define DISPLAY_HEIGHT DISPLAY_WIDTH
#define D_SCALE 0.5

#define DAC_MAX 0x1000
#define D_OFFSET_X ((DISPLAY_WIDTH - (DAC_MAX * D_SCALE)) / 2)
#define D_OFFSET_Y ((DISPLAY_HEIGHT - (DAC_MAX * D_SCALE)) / 2)

#define MAX_DL_SIZE 4096
static draw_list_t dl[MAX_DL_SIZE];
static unsigned n_dl = 0;

// Count how many samples were outputted
unsigned n_samples = 0;

SDL_Renderer *rr = NULL;
SDL_Window* window = NULL;

void _putchar(char c)
{
	push_char(c, 7, 100);
}

static void read_csv(char *fName)
{
	FILE* f = fopen(fName, "r");
	int a, b, c, d;
	draw_list_t *p = dl;
	char line[1024];
	char *tmp = NULL;

	n_dl = 0;
	while (fgets(line, 1024, f)) {
		if (line[0] == '#' || strlen(line) < 8)
			continue;

		tmp = strtok(line, ",");
		if (tmp == NULL) continue;
		a = atoi(tmp);

		tmp = strtok(NULL, ",");
		if (tmp == NULL) continue;
		b = atoi(tmp);

		tmp = strtok(NULL, ",");
		if (tmp == NULL) continue;
		c = atoi(tmp);

		tmp = strtok(NULL, ",");
		if (tmp == NULL) continue;
		d = atoi(tmp);

		// printf("%d %d %d %d\n", a, b, c, d);
		p->type = a;
		p->density = b;
		p->x = c << FP;
		p->y = d << FP;
		p++;
		n_dl++;

		if (n_dl >= MAX_DL_SIZE)
			break;
	}
	printf("read %d entries\n", n_dl);
	fclose(f);
}

static void demo_circles(unsigned frame)
{
	push_circle(
		-800, -800,
		100, 100,
		frame * (MAX_ANGLE >> 8),  // 0
		MAX_ANGLE / 4,  // MAX_ANGLE
		100
	);
	push_circle(
		-800, 800,
		100, 100,
		frame * (MAX_ANGLE >> 8),  // 0
		2 * MAX_ANGLE / 4,  // MAX_ANGLE
		100
	);
	push_goto(800 << FP, -800 << FP);
	push_circle(
		800, -800,
		100, 100,
		frame * (MAX_ANGLE >> 8),  // 0
		3 * MAX_ANGLE / 4,  // MAX_ANGLE
		100
	);
	push_circle(
		800, 800,
		100, 100,
		0,  // 0
		(sin(frame / 50.0) + 1) * MAX_ANGLE / 2,  // MAX_ANGLE
		100
	);
}

static void demo_text(unsigned frame)
{
	push_goto(0, 300);
	push_str(
		"Hello World 123\nWahUuiUuiu!\nit's drawing!\n;)",
		A_CENTER,
		(sin(frame / 50.0) + 1.01) * 1000,
		150
	);
}

// Visualize a sample, emulate the phosphor with additive blending
void push_sample(uint16_t val_a, uint16_t val_b, uint16_t val_c, uint16_t val_d)
{
	static int x_ = 0;
	static int y_ = 0;

	// center and scale the DAC range to the window-size
	int x = val_a * D_SCALE + D_OFFSET_X;
	int y = val_b * D_SCALE + D_OFFSET_Y;

	// Vertical mirror
	y = DISPLAY_HEIGHT - y;

	// The longer the distance between 2 points, the lower the intensity
	float len = sqrt((x - x_) * (x - x_) + (y - y_) * (y - y_));
	int intens = val_c / len / 2;
	if (intens < 0x40)
		intens = 0x40;
	if (intens > 0xFF)
		intens = 0xFF;

	// Draw lines connecting the samples. Red for blanked moves
	if (val_c == 0) {
		SDL_SetRenderDrawColor(rr, intens, 0x00, 0x00, 0xFF);
	} else {
		SDL_SetRenderDrawColor(rr, 0x00, intens, 0x00, 0xFF);
	}
	SDL_RenderDrawLine(rr, x_, y_, x, y);

	// Draw dots where the samples actually are to show the density
	if (val_c == 0) {
		SDL_SetRenderDrawColor(rr, 0x80, 0x00, 0x00, 0xFF);
	} else {
		SDL_SetRenderDrawColor(rr, 0x60, 0x60, 0x60, 0xFF);
	}
	SDL_Rect rect = {x - 2, y - 2, 5, 5};
	SDL_RenderFillRect(rr, &rect);
	x_ = x;
	y_ = y;
	n_samples++;
}

static void init_sdl()
{
	srand(time(NULL));

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
		return;
	}

	if (SDL_CreateWindowAndRenderer(
		DISPLAY_WIDTH, DISPLAY_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &rr
	)) {
		fprintf(stderr, "could not create window: %s\n", SDL_GetError());
		return;
	}

	// SDL_RenderSetScale(rr, ZOOM, ZOOM);
	SDL_SetRenderDrawBlendMode(rr, SDL_BLENDMODE_ADD);
}

int main(int argc, char* args[])
{
	if (argc != 2) {
		printf("also try %s draw_list.csv\n", args[0]);
	} else {
		printf("reading %s\n", args[1]);
		read_csv(args[1]);
	}

	init_lut();
	init_sdl();

	unsigned frame = 0;
	unsigned demo = 0;
	while (1) {
		SDL_Event e;
		bool isExit = false;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_QUIT:
					isExit = true;
					break;
				case SDL_KEYDOWN:
					switch(e.key.keysym.sym) {
						case SDLK_LEFT:
							demo--;
							break;
						case SDLK_RIGHT:
							demo++;
							break;
						case SDLK_DOWN:
							break;
						case SDLK_UP:
							break;
					}
					break;
			}
		}
		if (isExit)
			break;

		SDL_SetRenderDrawColor(rr, 0x00, 0x00, 0x00, 0xFF);
		SDL_RenderClear(rr);

		push_list(dl, n_dl);

		switch (demo % 3) {
		case 1:
			demo_circles(frame);
			break;
		case 2:
			demo_text(frame);
			break;
		}

		if (frame < 200) {
			push_goto(0, -800);
			push_str("<--  -->", A_CENTER, 200, 200 - frame);
		}

		printf("%d\n", n_samples);
		n_samples = 0;

		SDL_RenderPresent(rr);
		SDL_Delay(50);
		frame++;
		// return 0;
	}

	SDL_DestroyRenderer(rr);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

