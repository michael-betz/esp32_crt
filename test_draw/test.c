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

#define N_DDS 4
static uint32_t phases[N_DDS] = {0, 0, 0, 0};
static uint32_t lut_type[N_DDS] = {1, 0, 3, 0};
static uint32_t l_shifts[N_DDS] = {0, 0, 0, 0};
static uint32_t delta_fs[N_DDS] = {
	// Carrier
	0x57301000, 0x57300000,
	// Modulator
	0x47303010, 0x47301200,
};

static void demo_dds(unsigned frame)
{
	int32_t tmps[N_DDS], x_val, y_val;

	for (unsigned s_i = 0; s_i < 3000; s_i++) {
		for (unsigned dds_i = 0; dds_i < N_DDS; dds_i++) {
			phases[dds_i] += delta_fs[dds_i];
			switch (lut_type[dds_i]) {
			case 0:
				tmps[dds_i] = get_sin(phases[dds_i] >> 20);
				break;
			case 1:
				tmps[dds_i] = get_cos(phases[dds_i] >> 20);
				break;
			case 2:
				tmps[dds_i] = phases[dds_i] - INT_MIN;
				break;
			case 3:
				tmps[dds_i] = (phases[dds_i] > (INT_MAX / 2)) ? INT_MAX : INT_MIN;
				break;
			case 4:
				tmps[dds_i] = INT_MAX;
				break;
			}
			tmps[dds_i] >>= l_shifts[dds_i];
		}

		// Amplitude modulation
		// x_val = (tmps[0] >> 16) * (tmps[2] >> 16);
		// y_val = (tmps[1] >> 16) * (tmps[3] >> 16);
		x_val = ((int64_t)tmps[0] * (int64_t)tmps[2]) >> 32;
		y_val = ((int64_t)tmps[1] * (int64_t)tmps[3]) >> 32;


		// normalize for full amplitude
		x_val >>= 20 - (l_shifts[0] + l_shifts[2]);
		y_val >>= 20 - (l_shifts[1] + l_shifts[3]);
		output_sample(x_val, y_val, 0x800, 0);
	}
	printf("%08x %08x %08x %08x | %08x %08x\n", tmps[0], tmps[1], tmps[2], tmps[3], x_val, y_val);
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
		printf("also try %s draw_list.bin\n", args[0]);
		*dl = T_END;
		n_dl = 1;
	} else {
		printf("reading %s\n", args[1]);
		read_dl(args[1]);
	}

	init_lut();
	init_sdl();

	unsigned frame = 0;
	int demo = 2;
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

