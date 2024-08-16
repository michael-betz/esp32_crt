// demo the drawing engine on a PC using SDL2
#include <stdbool.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <cjson/cJSON.h>
#include "draw.h"
#include "font_draw.h"
#include "wireframe_draw.h"
#include "fonts/font_data.h"
#include "fast_sin.h"
#include "dds.h"
#include "demo_mode.h"
#include "meteo_swiss.h"
#include "json_settings.h"

#define DISPLAY_WIDTH 1024
#define DISPLAY_HEIGHT DISPLAY_WIDTH
#define D_SCALE 0.25

#define DAC_MAX 0x1000
#define D_OFFSET_X ((DISPLAY_WIDTH - (DAC_MAX * D_SCALE)) / 2)
#define D_OFFSET_Y ((DISPLAY_HEIGHT - (DAC_MAX * D_SCALE)) / 2)

// Count how many samples were outputted
unsigned n_samples = 0;

SDL_Renderer *rr = NULL;
SDL_Window* window = NULL;

// Visualize a sample, emulate the phosphor with additive blending
void push_sample(uint16_t val_x, uint16_t val_y, uint16_t val_blank, uint16_t val_foc)
{
	static int x_ = 0;
	static int y_ = 0;

	// printf("(%6d, %6d, %6d)\n", val_x, val_y, val_blank);
	// assert(val_x < 0x1000);
	// assert(val_y < 0x1000);

	// center and scale the DAC range to the window-size
	int x = val_x * D_SCALE + D_OFFSET_X;
	int y = val_y * D_SCALE + D_OFFSET_Y;

	// Vertical mirror
	y = DISPLAY_HEIGHT - y;

	// The longer the distance between 2 points, the lower the intensity
	float len = sqrt((x - x_) * (x - x_) + (y - y_) * (y - y_));
	int intens = 10000 / len / 2;
	if (intens < 0x02)
		intens = 0x02;
	if (intens > 0xFF)
		intens = 0xFF;

	// Draw lines connecting the samples. Red for blanked moves
	if (val_blank >= 0x800) {
		SDL_SetRenderDrawColor(rr, 0xFF, 0x00, 0x00, 0x80);
	} else {
		SDL_SetRenderDrawColor(rr, 0x00, intens, 0x00, 0xFF);
	}
	SDL_RenderDrawLine(rr, x_, y_, x, y);

	// Draw dots where the samples actually are to show the density
	if (val_blank >= 0x800) {
		SDL_SetRenderDrawColor(rr, 0xFF, 0x00, 0x00, 0x40);
	} else {
		SDL_SetRenderDrawColor(rr, 0xFF, 0xFF, 0xFF, 0x40);
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

int encoder_value = 0;

int get_encoder()
{
	return encoder_value;
}

int main(int argc, char* args[])
{
	init_lut();
	init_sdl();
	setup_dds(0x070F0300, 0x070F0400, 0x07000000, 0x07000700, 0x1012);

	cJSON *weather = readJsonDyn("meteo_data.json");
	if (weather == NULL) {
		printf("reading meteo_data.json failed\n");
		return -1;
	}
	weather_set_json(weather);

	unsigned frame = 0;
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
							encoder_value--;
							break;
						case SDLK_RIGHT:
							encoder_value++;
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

		demo_mode();

		// printf("%d\n", n_samples);
		n_samples = 0;

		SDL_RenderPresent(rr);
		SDL_Delay(20);
		frame++;
	}

	cJSON_Delete(weather);

	SDL_DestroyRenderer(rr);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

