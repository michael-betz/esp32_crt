#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "fonts/font_data.h"
#include "draw.h"
#include "wireframe_draw.h"
#include "meteo_swiss.h"
#include "fast_sin.h"
#include "dds.h"
#include "encoder.h"


static void demo_text(unsigned font)
{
	char tmp_str[64];
	static int frame = 0;

	font %= N_FONTS;
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
    strftime(tmp_str, sizeof(tmp_str), "%A\n%d.%m.%y\n%k:%M:%S", &timeinfo);

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


static void test_image()
{
	time_t now;
	struct tm timeinfo;
	char tmp_str[16];

	time(&now);
	localtime_r(&now, &timeinfo);
    strftime(tmp_str, sizeof(tmp_str), "%k:%M:%S", &timeinfo);

	// a square around the screen
	push_goto(-2000, -2000);
	push_line(-2000, 2000, 30);
	push_line(2000, 2000, 30);
	push_line(2000, -2000, 30);
	push_line(-2000, -2000, 30);

	// inner cross
	// push_goto(-200, -200);
	// push_line(200, 200, 255);
	// push_goto(-200, 200);
	// push_line(200, -200, 255);

	// inner +
	// push_goto(-500, 0);
	// push_line(500, 0, 50);
	// push_goto(0, 500);
	// push_line(0, -500, 50);

	// Draw some filled boxes to simulate a QR code
	const int N = 33;
	int x = 0;
	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			if ((i * j * now) & 32)
				draw_filled_box((x - N / 2) * 70, (-i + N / 2 + 6) * 70, 50, 100);
			if (j < N - 1) {
				if (i & 1)
					x--;
				else
					x++;
			}
		}
	}

	// concentric circles
	// for (unsigned i=1; i<=10; i++) {
	// 	push_circle(
	// 		0,
	// 		0,
	// 		i * 200,
	// 		i * 200,
	// 		i <= 5 ? 0 : -280,
	// 		i <= 5 ? MAX_ANGLE : MAX_ANGLE - 1500,
	// 		100
	// 	);
	// }

	set_font_index(0);
	push_str(0, -1800, tmp_str, sizeof(tmp_str), A_CENTER, 900, 200);
}

void demo_mode()
{
	static int demo_text_font = 0, mode = 0;
	static time_t ticks_ = 0;
	static int enc_ = 0;

	switch (mode) {
		case 0:
			test_image();
			break;

		case 1:
			wf_test();
			break;

		case 2:
			demo_text(demo_text_font);
			break;

		case 3:
			draw_dds(5000);
			nudge_dds();
			break;

		case 4:
			draw_weather_grid();
			break;

		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			rain_temp_plot(mode - 5);
			break;

		default:
			if (mode < 0) {
				mode = 10;
				demo_text_font--;
			} else {
				mode = 0;
				demo_text_font++;
			}
	}

	int enc = get_encoder();
	mode += enc - enc_;
	enc_ = enc;

	time_t ticks = time(NULL);
	if ((ticks - ticks_) > 60) {
		mode++;
		ticks_ = ticks;
	}
}
