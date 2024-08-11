// demo the drawing engine on a PC using SDL2
#include <stdbool.h>
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
#include "demo_mode.h"
#include "meteo_swiss.h"
#include <curl/curl.h>
#include <cjson/cJSON.h>

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

static void demo_text(unsigned frame, unsigned font)
{
	char tmp_str[64];
	if (font >= N_FONTS)
		return;
	snprintf(tmp_str, sizeof(tmp_str), "f: %d", font);

	set_font_index(0);
	push_str(
		-1300, 1300,
		tmp_str,
		sizeof(tmp_str),
		A_LEFT,
		300,
		200
	);

	time_t now;
	struct tm timeinfo;

	time(&now);
	localtime_r(&now, &timeinfo);
	strftime(tmp_str, sizeof(tmp_str), "%A\n%d.%m.%y\n%k:%M:%S", &timeinfo);

	int font_size = ((get_sin(frame++ * MAX_ANGLE / 5000) >> 16) + (1 << 15)) * 1000 / (1 << 16) + 50;
	set_font_index(font);
	push_str(
		0, 500,
		tmp_str,
		sizeof(tmp_str),
		A_CENTER,
		font_size,
		300
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

	// // inner cross
	push_goto(-200, -200);
	push_line(200, 200, 100);
	push_goto(-200, 200);
	push_line(200, -200, 100);

	// // inner +
	push_goto(-500, 0);
	push_line(500, 0, 50);
	push_goto(0, 500);
	push_line(0, -500, 50);

	// concentric circles
	for (unsigned i=3; i<=10; i++) {
		push_circle(
			0,
			0,
			i * 200,
			i * 200,
			i <= 5 ? i * 256 : -280,
			i <= 5 ? MAX_ANGLE : MAX_ANGLE - 1500,
			10
		);
	}

	time_t now;
	time(&now);

	struct tm timeinfo;
	localtime_r(&now, &timeinfo);
	char tmp_str[16];
	strftime(tmp_str, sizeof(tmp_str), "%H:%M:%S", &timeinfo);

	set_font_index(0);
	push_str(0, -1800, tmp_str, sizeof(tmp_str), A_CENTER, 900, 200);
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

	// printf("(%6d, %6d, %6d)\n", val_x, val_y, val_blank);
	assert(val_x < 0x1000);
	assert(val_y < 0x1000);

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
	if (val_blank >= 0x800) {
		SDL_SetRenderDrawColor(rr, 0xFF, 0x00, 0x00, 0x00);
	} else {
		SDL_SetRenderDrawColor(rr, 0x00, 0xFF, 0x00, 0xFF);
	}
	SDL_RenderDrawLine(rr, x_, y_, x, y);

	// Draw dots where the samples actually are to show the density
	if (val_blank >= 0x800) {
		SDL_SetRenderDrawColor(rr, 0x80, 0x00, 0x00, 0xFF);
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



struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
	/* out of memory! */
	printf("not enough memory (realloc returned NULL)\n");
	return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int get_weather(unsigned postcode, cJSON **weather)
{
	char url[128];
	snprintf(url, sizeof(url), "https://app-prod-ws.meteoswiss-app.ch/v1/plzDetail?plz=%d", postcode * 100);
	printf("GET %s\n", url);

	struct MemoryStruct chunk;
	chunk.memory = malloc(1);  /* grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */

	CURL *curl = curl_easy_init();
	CURLcode res;

	/* send all data to this function  */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	curl_easy_setopt(curl, CURLOPT_URL, url);

	res = curl_easy_perform(curl);
	if(res != CURLE_OK) {
		printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		return res;
	}

	printf("%lu bytes retrieved\n", (unsigned long)chunk.size);

	*weather = cJSON_ParseWithLength(chunk.memory, chunk.size);

	free(chunk.memory);
	curl_easy_cleanup(curl);

	return *weather == NULL ? -1 : 0;
}

int main(int argc, char* args[])
{
	init_lut();
	init_sdl();
	setup_dds(0x070F0300, 0x070F0400, 0x07000000, 0x07000700, 0x1012);

	cJSON *weather = NULL;
	int ret = get_weather(1202, &weather);
	if (ret < 0) {
		printf("get_weather() failed %d\n", ret);
		return ret;
	}

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

		draw_weather_icons(weather, demo);

		// demo_mode();

		// if (demo < 0)
		// 	demo = N_FONTS + 3;

		// if (demo > N_FONTS + 3)
		// 	demo = 0;

		// switch (demo) {
		// case 0:
		// 	test_image();
		// 	break;
		// case 1:
		// 	demo_circles(frame);
		// 	break;
		// case 2:
		// 	demo_dds(frame);
		// 	break;
		// case 3:
		// 	wf_test();
		// 	break;
		// default:
		// 	demo_text(frame, demo - 4);
		// 	break;
		// }

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

