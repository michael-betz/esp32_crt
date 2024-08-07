#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "fonts/font_data.h"
#include "draw.h"
#include "wireframe_draw.h"
#include "fast_sin.h"
#include "dds.h"


static void demo_text(unsigned font)
{
	char tmp_str[64];
	static int frame = 0;

	if (font >= N_FONTS)
		return;
	snprintf(tmp_str, sizeof(tmp_str), "f: %d", font);

	set_font(0);
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
	set_font(font);
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
	// a square around the screen
	push_goto(-2000, -2000);
	push_line(-2000, 2000, 30);
	push_line(2000, 2000, 30);
	push_line(2000, -2000, 30);
	push_line(-2000, -2000, 30);

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
	for (unsigned i=1; i<=10; i++) {
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

	time_t now;
	struct tm timeinfo;
	char tmp_str[16];

	time(&now);
	localtime_r(&now, &timeinfo);
    strftime(tmp_str, sizeof(tmp_str), "%k:%M:%S", &timeinfo);

	set_font(0);
	push_str(0, -1800, tmp_str, sizeof(tmp_str), A_CENTER, 900, 400);
}

void demo_mode()
{
	static int demo_text_font = 0, mode = 0;
	static time_t ticks_ = 0;

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

		default:
			mode = 0;
			demo_text_font++;
			if (demo_text_font > N_FONTS)
				demo_text_font = 0;
	}

	time_t ticks = time(NULL);

	if ((ticks - ticks_) > 20) {
		mode++;
		ticks_ = ticks;
	}
}
