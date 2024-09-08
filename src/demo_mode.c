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

static void demo_text(unsigned *enc)
{
	char tmp_str[64];
	static int frame=0, font=0;

	if (encoder_helper(enc, &font, 0, N_FONTS - 2))
		return;

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
}

static void square_wave()
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
}

static void test_image(unsigned *enc)
{
	static int n_circles = 3;

	if (encoder_helper(enc, &n_circles, 4, 10))
		return;

	printf("%d\n", n_circles);

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
	for (unsigned i=n_circles; i<=10; i++) {
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
}

void demo_mode()
{
	static int mode = 0;
	static time_t ticks_ = 0;

	unsigned enc = 0;
	get_encoder(&enc);

	switch (mode) {
		case 0:
			// square_wave();
			test_image(&enc);
			break;

		case 1:
			wf_test();
			break;

		case 2:
			demo_text(&enc);
			break;

		case 3:
			draw_dds(5000);
			nudge_dds();
			break;

		case 4:
			draw_weather_grid();
			break;

		case 5:
			rain_temp_plot(&enc);
			break;

		case 6:
			meteo_radar();
			break;

		default:
			if (mode < 0)
				mode = 6;
			else
				mode = 0;
	}

	mode += (int8_t)(enc >> 24);

	if (ticks_ == 0)
		ticks_ = time(NULL);

	time_t ticks = time(NULL);
	if (ticks - ticks_ > 60) {
		mode++;
		ticks_ = ticks;
	}
}
