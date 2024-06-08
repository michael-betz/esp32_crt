// demo the drawing engine on a PC using SDL2
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include "draw.h"
#include "font_draw.h"
#include "font_data.h"
#include "fast_sin.h"
#include "dds.h"

#include "test_3d.h"

#define DISPLAY_WIDTH 1024
#define DISPLAY_HEIGHT DISPLAY_WIDTH
#define D_SCALE 0.5

#define DAC_MAX 0x1000
#define D_OFFSET_X ((DISPLAY_WIDTH - (DAC_MAX * D_SCALE)) / 2)
#define D_OFFSET_Y ((DISPLAY_HEIGHT - (DAC_MAX * D_SCALE)) / 2)

// #define MAX_DL_SIZE 4096
// static uint8_t dl[MAX_DL_SIZE];
// static unsigned n_dl = 0;  // bytes in dl

// Count how many samples were outputted
unsigned n_samples = 0;

SDL_Renderer *rr = NULL;
SDL_Window* window = NULL;


// static void read_dl(char *fName)
// {
// 	FILE *f = NULL;
// 	if (strcmp(fName, "-") == 0) {
// 		f = stdin;
// 	} else {
// 		f = fopen(fName, "r");
// 		if (f == NULL)
// 			perror("Couldn't open input file");
// 	}

// 	int n = fread(dl, 1, MAX_DL_SIZE, f);
// 	fclose(f);

// 	if (n <= 0)
// 		perror("Couldn't read from input file");
// 	n_dl = n;
// 	printf("draw_list of %d bytes\n", n_dl);
// }

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

static void demo_text(unsigned frame, unsigned font)
{
	char tmp[32];
	if (font >= N_FONTS)
		return;
	snprintf(tmp, sizeof(tmp), "font: %d", font);

	set_font(0);
	push_str(
		-970, 950,
		tmp,
		sizeof(tmp),
		A_LEFT,
		100,
		200
	);

	set_font(font);
	push_str(
		0, 500,
		"esp_crt\nHello World\n1234:5678",
		// "p",
		128,
		A_CENTER,
		// 18,
		(sin(frame / 200.0) + 1.01) * 500,
		100
	);
	// exit(0);
}

static void test_image()
{
	// a square around the screen
	push_goto(-2000, -2000);
	push_line(-2000, 2000, 10);
	push_line(2000, 2000, 10);
	push_line(2000, -2000, 10);
	push_line(-2000, -2000, 10);

	// inner cross
	push_goto(-200, -200);
	push_line(200, 200, 255);
	push_goto(-200, 200);
	push_line(200, -200, 255);

	// inner +
	push_goto(-500, 0);
	push_line(500, 0, 50);
	push_goto(0, 500);
	push_line(0, -500, 50);

	// concentric circles
	push_circle(0, 0, 100, 100, 0, MAX_ANGLE, 10);
	push_circle(0, 0, 200, 200, 0, MAX_ANGLE, 30);
	push_circle(0, 0, 300, 300, 0, MAX_ANGLE, 50);
	push_circle(0, 0, 400, 400, 0, MAX_ANGLE, 70);
	push_circle(0, 0, 500, 500, 0, MAX_ANGLE, 90);
	push_circle(0, 0, 600, 600, 0, MAX_ANGLE, 110);
	push_circle(0, 0, 700, 700, 3341, 3534, 130);
	push_circle(0, 0, 800, 800, 3341, 3534, 150);
	push_circle(0, 0, 900, 900, 3341, 3534, 170);

	set_font(0);
	push_str(0, -775, "<- ->\nHello World", 18, A_CENTER, 200, 200);
}

static void demo_dds(unsigned frame)
{
	// DAC \Delta t is 1.6 us
	// SDL frame is 50 ms
	// Need to draw 31k samples per frame
	draw_dds(8000);
	if ((frame % 10) == 0) {
		printf("nudge!!!");
		nudge_dds();
	}
}

// Visualize a sample, emulate the phosphor with additive blending
void push_sample(uint16_t val_x, uint16_t val_y, uint16_t val_blank, uint16_t val_foc)
{
	static int x_ = 0;
	static int y_ = 0;

	// center and scale the DAC range to the window-size
	int x = val_x * D_SCALE + D_OFFSET_X;
	int y = val_y * D_SCALE + D_OFFSET_Y;

	// Vertical mirror
	y = DISPLAY_HEIGHT - y;

	// The longer the distance between 2 points, the lower the intensity
	float len = sqrt((x - x_) * (x - x_) + (y - y_) * (y - y_));
	int intens = 2000 / len / 2;
	if (intens < 0x02)
		intens = 0x02;
	if (intens > 0xFF)
		intens = 0xFF;

	// Draw lines connecting the samples. Red for blanked moves
	if (val_blank >= 0xF00) {
		SDL_SetRenderDrawColor(rr, 0x40, 0x00, 0x00, 0xFF);
	} else {
		SDL_SetRenderDrawColor(rr, 0x00, intens, 0x00, 0xFF);
	}
	SDL_RenderDrawLine(rr, x_, y_, x, y);

	// Draw dots where the samples actually are to show the density
	if (val_blank >= 0xF00) {
		SDL_SetRenderDrawColor(rr, 0x80, 0x00, 0x00, 0x80);
	} else {
		SDL_SetRenderDrawColor(rr, 0x60, 0x60, 0x60, 0x80);
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
	// if (argc != 2) {
	// 	printf("also try %s draw_list.bin\n", args[0]);
	// 	*dl = T_END;
	// 	n_dl = 1;
	// } else {
	// 	printf("reading %s\n", args[1]);
	// 	read_dl(args[1]);
	// }

	init_lut();
	init_sdl();
	setup_dds(0x070F0300, 0x070F0400, 0x07000000, 0x07000700, 0x1012);

	unsigned frame = 0;
	int demo = 5;
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

		// push_list(dl, n_dl);

		if (demo < 0)
			demo = N_FONTS + 3;

		if (demo > N_FONTS + 3)
			demo = 0;

		switch (demo) {
		case 0:
			test_image();
			break;
		case 1:
			demo_circles(frame);
			break;
		case 2:
			demo_dds(frame);
			break;
		case 3:
			draw_mesh();
			break;
		default:
			demo_text(frame, demo - 4);
			break;
		}

		printf("%d\n", n_samples);
		n_samples = 0;

		SDL_RenderPresent(rr);
		SDL_Delay(20);
		frame++;
		// return 0;
	}

	SDL_DestroyRenderer(rr);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

