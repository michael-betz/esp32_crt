// demo the drawing engine on a PC using SDL2
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include "draw.h"
#include "font_draw.h"
#include "wireframe_draw.h"
#include "fonts/font_data.h"
#include "fast_sin.h"
#include "dds.h"
#include "screen_handler.h"
#include "meteo_swiss.h"

#ifdef __EMSCRIPTEN__
	#include "emscripten.h"
	#include "emscripten/html5.h"
#endif

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

char qr_code_str[32];
char *qr_code = NULL;
unsigned qr_code_w = 24;

void every_second()
{
	if (qr_code == NULL)
		qr_code = malloc(qr_code_w * qr_code_w / 8);

	char *p = qr_code;
	int n_bits = 0;
	unsigned tmp = 0;
	time_t now = 0;
	static time_t now_ = 0;
	struct tm timeinfo;

	time(&now);

	if (now == now_)
		return;

	localtime_r(&now, &timeinfo);
	strftime(qr_code_str, sizeof(qr_code_str), "%H:%M:%S", &timeinfo);

	for (int y = 0; y < qr_code_w; y++) {
		for (int x = 0; x < qr_code_w; x++) {
			if ((1 + x * y * now) & 64)
				tmp |= 1 << n_bits;
			if (n_bits++ >= 7) {
				*p++ = tmp;
				tmp = 0;
				n_bits = 0;
			}
		}
	}
	now_ = now;
}


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
		SDL_SetRenderDrawColor(rr, 0xFF, 0x00, 0x00, 0x60);
	} else {
		SDL_SetRenderDrawColor(rr, 0x00, intens, 0x00, 0xFF);
	}
	SDL_RenderDrawLine(rr, x_, y_, x, y);

	// Draw dots where the samples actually are to show the density
	if (val_blank >= 0x800) {
		SDL_SetRenderDrawColor(rr, 0xFF, 0x00, 0x00, 0x40);
	} else {
		SDL_SetRenderDrawColor(rr, 0xFF, 0xFF, 0xFF, 0x20);
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

static int8_t encoder_value = 0;

int8_t get_encoder(unsigned *state)
{
    static unsigned btn_state_ = 0;
	const uint8_t *keys = SDL_GetKeyboardState(NULL);

    if (state != NULL) {
		unsigned btn_state = (keys[SDL_SCANCODE_DOWN] << 1) | keys[SDL_SCANCODE_UP];

        unsigned rising = (~btn_state_) & btn_state;
        unsigned falling = btn_state_ & (~btn_state);
        *state = (((int8_t)(encoder_value)) << 24) | (falling << 16) | (rising << 8) | btn_state;
        btn_state_ = btn_state;
    }

	return encoder_value;
}

bool is_running = true;

void one_iter()
{
	SDL_Event e;
	encoder_value = 0;

	while (SDL_PollEvent(&e)) {
		switch (e.type) {
			case SDL_QUIT:
				is_running = false;
				#ifdef __EMSCRIPTEN__
					emscripten_cancel_main_loop();
				#endif
				break;
			case SDL_KEYDOWN:
				switch(e.key.keysym.sym) {
					case SDLK_LEFT:
						encoder_value = -1;
						break;
					case SDLK_RIGHT:
						encoder_value = 1;
						break;
				}
				break;
		}
	}

	SDL_SetRenderDrawColor(rr, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderClear(rr);

	every_second();
	screen_handler();
	SDL_RenderPresent(rr);
}

#ifdef __EMSCRIPTEN__
EM_BOOL touch_callback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
	printf("touch [%d] ", touchEvent->numTouches);
	for (int i=0; i<touchEvent->numTouches; i++) {
		printf("(%d, %d) ", touchEvent->touches[i].clientX, touchEvent->touches[i].clientY);
	}
	printf("\n");
	return true;
}
#endif

int main(int argc, char* args[])
{
	init_lut();
	init_sdl();
	setup_dds(0x070F0300, 0x070F0400, 0x07000000, 0x07000700, 0x1012);

	request_weather_data();

	#ifdef __EMSCRIPTEN__
		emscripten_set_touchstart_callback("#canvas", NULL, false, touch_callback);
		emscripten_set_main_loop(one_iter, 0, 1);
	#else
		while (is_running) {
			one_iter();
			SDL_Delay(20);
		}

		SDL_DestroyRenderer(rr);
		SDL_DestroyWindow(window);
		SDL_Quit();
	#endif

	return 0;
}
