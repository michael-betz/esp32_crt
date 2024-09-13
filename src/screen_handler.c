#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "fonts/font_data.h"
#include "draw.h"
#include "wireframe_draw.h"
#include "meteo_swiss.h"
#include "meteo_radar.h"
#include "fast_sin.h"
#include "dds.h"
#include "encoder.h"
#include "main.h"
#include "i2s.h"
#include "screen_handler.h"

typedef struct {
	const int par_min;
	const int par_max;
	int (*functionPtr)(int);
} t_screen;

static int current_screen = 0, current_par = 0;

// Define the list of screens to show here ...
static const t_screen screens[] = {
	{0,  6 * 4, demo_text},
	{5, 10, test_image},
	{0,  0, wf_test},
	{0,  0, draw_weather_grid},
	{5, 15, rain_temp_plot},
	{0, 16, draw_weather_symbol},
	// {0,  0, square_wave},
};
#define N_SCREENS (sizeof(screens) / sizeof(screens[0]))


static void inc_screen()
{
	current_screen++;
	if (current_screen >= N_SCREENS)
		current_screen = 0;
	current_par = screens[current_screen].par_min;
}

static void dec_screen()
{
	current_screen--;
	if (current_screen < 0)
		current_screen = N_SCREENS - 1;
	current_par = screens[current_screen].par_max;
}

void screen_handler()
{
	static time_t ticks_ = 0;
	if (ticks_ == 0) {
		ticks_ = time(NULL);
		current_screen = 0;
		current_par = screens[current_screen].par_min;
	}

	unsigned enc = 0;
	get_encoder(&enc);
	int8_t diff = (int8_t)(enc >> 24);

	// The encoder switches between screens, so we need to select one of the
	// drawing functions. Each screen has a parameter which also changes
	// with encoder position. If the parameter reaches its max / min value
	// the screen shall be switched

	// Handle all accumulated encoder ticks
	while (diff != 0) {
		if (diff > 0) {
			current_par++;
			if (current_par > screens[current_screen].par_max)
				inc_screen();
			diff--;
		} else {
			current_par--;
			if (current_par < screens[current_screen].par_min)
				dec_screen();
			diff++;
		}
	}

	// Auto advance screens every N seconds
	time_t ticks = time(NULL);
	if (ticks - ticks_ > 60) {
		inc_screen();
		ticks_ = ticks;
	}

	// Actually draw the screens
	int ret = screens[current_screen].functionPtr(current_par);
	if (ret < 0) {
		printf("Warning, screen_%d(%d) returned %d\n", current_screen, current_par, ret);
		inc_screen();
	}
}


// ----------------------------------
//  Some more demo screens for fun
// ----------------------------------

int demo_text(int font)
{
	char tmp_str[64];
	static int frame=0;

	set_rotation((12 - font) * 8);

	font /= 4;

	snprintf(tmp_str, sizeof(tmp_str), "f: %d", font);

	set_font_name(NULL);
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
	strftime(tmp_str, sizeof(tmp_str), "%A\n%d.%m.%y\n%H:%M:%S", &timeinfo);

	int font_size = ((get_sin(frame++ * MAX_ANGLE / 5000) >> 16) + (1 << 15)) * 1000 / (1 << 16) + 200;
	set_font_index(font);
	push_str(
		0, 500,
		tmp_str,
		sizeof(tmp_str),
		A_CENTER,
		font_size,
		200
	);

	set_rotation(0);

	return 0;
}

int square_wave(int par)
{
	// a square-wave amplifier bandwidth test pattern
	const int n = 0x100;

	for (unsigned i=0; i<n; i++)
		push_sample(0, 0, 0, 0);

	for (unsigned i=0; i<n; i++)
		push_sample(0xFFF, 0, 0, 0);

	for (unsigned i=0; i<n; i++)
		push_sample(0xFFF, 0xFFF, 0, 0);

	for (unsigned i=0; i<n; i++)
		push_sample(0xFFF, 0xFFF, 0xFFF, 0);

	for (unsigned i=0; i<n; i++)
		push_sample(0xFFF, 0xFFF, 0xFFF, 0xFFF);

	for (unsigned i=0; i<n; i++)
		push_sample(0, 0xFFF, 0xFFF, 0xFFF);

	for (unsigned i=0; i<n; i++)
		push_sample(0, 0, 0xFFF, 0xFFF);

	for (unsigned i=0; i<n; i++)
		push_sample(0, 0, 0, 0xFFF);

	for (unsigned i=0; i<n; i++)
		push_sample(0, 0, 0, 0);

	return 0;
}

int test_image(int n_circles)
{
	// a square around the screen
	push_goto(-2040, -2040);
	push_line(-2040, 2040, 30);
	push_line(2040, 2040, 30);
	push_line(2040, -2040, 30);
	push_line(-2040, -2040, 30);

	// // inner cross
	// push_goto(-200, -200);
	// push_line(200, 200, 255);
	// push_goto(-200, 200);
	// push_line(200, -200, 255);

	// // inner +
	// push_goto(-500, 0);
	// push_line(500, 0, 50);
	// push_goto(0, 500);
	// push_line(0, -500, 50);

	// Draw a QR code
	if (qr_code_w > 0 && qr_code != NULL) {
		int size = qr_code_w, n_bits = 0;
		char *p = qr_code;
		unsigned tmp = *p++;
		for (int y = -1; y < size + 1; y++) {
			for (int x = -1; x < size + 1; x++) {
				if (x < 0 || y < 0 || x > (size - 1) || y > (size - 1)) {
					draw_filled_box((x - size / 2) * 50, (-y + size / 2) * 50, 30, 100);
				} else {
					if (!(tmp & (1 << n_bits))) {
						draw_filled_box((x - size / 2) * 50, (-y + size / 2) * 50, 30, 100);
					}
					if (n_bits++ >= 7) {
						tmp = *p++;
						n_bits = 0;
					}
				}
			}
		}
	}

	// concentric circles
	for (unsigned i=5; i<=n_circles; i++) {
		push_circle(
			0,
			0,
			i * 200,
			i * 200,
			i <= 5 ? 0 : -280,
			i <= 5 ? MAX_ANGLE : MAX_ANGLE - 1500,
			100
		);
	}

	set_font_name(NULL);
	push_str(0, -1800, qr_code_str, sizeof(qr_code_str), A_CENTER, 700, 200);
	return 0;
}
