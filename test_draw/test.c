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

#define MAX_DL_SIZE 4096
static uint8_t dl[MAX_DL_SIZE];
static unsigned n_dl = 0;  // bytes in dl

// Count how many samples were outputted
unsigned n_samples = 0;

SDL_Renderer *rr = NULL;
SDL_Window* window = NULL;


static void read_dl(char *fName)
{
	FILE *f = NULL;
	if (strcmp(fName, "-") == 0) {
		f = stdin;
	} else {
		f = fopen(fName, "r");
		if (f == NULL)
			perror("Couldn't open input file");
	}

	int n = fread(dl, 1, MAX_DL_SIZE, f);
	fclose(f);

	if (n <= 0)
		perror("Couldn't read from input file");
	n_dl = n;
	printf("draw_list of %d bytes\n", n_dl);
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

	set_font(9);
	push_str(
		-970, 950,
		tmp,
		sizeof(tmp),
		A_LEFT,
		300,
		100
	);

	set_font(font);
	push_str(
		0, 300,
		"esp_crt\nHello World\n12345678",
		1024,
		A_CENTER,
		(sin(frame / 50.0) + 1.01) * 1000,
		100
	);
}

static void demo_dds(unsigned frame)
{
	// DAC \Delta t is 1.6 us
	// SDL frame is 50 ms
	// Need to draw 31k samples per frame
	draw_dds(5000);
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
	int intens = val_blank / len / 2;
	if (intens < 0x30)
		intens = 0x30;
	if (intens > 0xFF)
		intens = 0xFF;

	// Draw lines connecting the samples. Red for blanked moves
	if (val_blank == 0) {
		SDL_SetRenderDrawColor(rr, intens, 0x00, 0x00, 0xFF);
	} else {
		SDL_SetRenderDrawColor(rr, 0x00, intens, 0x00, 0xFF);
	}
	SDL_RenderDrawLine(rr, x_, y_, x, y);

	// Draw dots where the samples actually are to show the density
	if (val_blank == 0) {
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
		printf("also try %s draw_list.bin\n", args[0]);
		*dl = T_END;
		n_dl = 1;
	} else {
		printf("reading %s\n", args[1]);
		read_dl(args[1]);
	}

	init_lut();
	init_sdl();
	setup_dds(0x070F0300, 0x070F0400, 0x07000000, 0x07000700, 0x1012);

	draw_mesh();

	unsigned frame = 0;
	int demo = 0;
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

		if (demo < 0)
			demo = N_FONTS + 2;

		if (demo > N_FONTS + 2)
			demo = 0;

		switch (demo) {
		case 0:
			break;
		case 1:
			demo_circles(frame);
			break;
		case 2:
			demo_dds(frame);
			break;
		default:
			demo_text(frame, demo - 3);
			break;
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

