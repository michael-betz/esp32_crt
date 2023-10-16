#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "i2s.h"
#include "draw.h"
#include "fast_sin.h"
#include "print.h"  // for printing fixed-point numbers

// current cursor position
static int16_t x_cur = 0;
static int16_t y_cur = 0;


// https://stackoverflow.com/questions/34187171/fast-integer-square-root-approximation
static unsigned usqrt4(unsigned val) {
    unsigned a, b;

    if (val < 2) return val; /* avoid div/0 */

    a = 1255;       /* starting point is relatively unimportant */

    b = val / a; a = (a+b) /2;
    b = val / a; a = (a+b) /2;
    b = val / a; a = (a+b) /2;
    b = val / a; a = (a+b) /2;

    return a;
}


static unsigned push_circle(int16_t r_x, int16_t r_y, uint8_t density)
{
	if (r_x == 0)
		r_x = r_y;
	if (r_y == 0)
		r_y = r_x;
	int r_max = r_x > r_y ? r_x : r_y;
	unsigned n = (2 * r_max * density * (unsigned)(M_PI * 64) / 64) >> (FP + 11);
	// printf("r: %d, density: %d, n: %d\n", r >> FP, density, n);

	int d_alpha = (MAX_ANGLE << 8) / n;

	for (unsigned i = 0; i < n; i++) {
		int arg = (i * d_alpha) >> 8;
		push_sample(
			x_cur + (((get_sin(arg) >> 16) * r_x) >> (FP + 11)),
			y_cur + (((get_cos(arg) >> 16) * r_y) >> (FP + 11)),
			0,
			0
		);
	}
	return n;
}


static unsigned push_line(int16_t x, int16_t y, uint8_t density)
{
	unsigned n_samples = 0;

	// local copies
	int x_ = x_cur;
	int y_ = y_cur;

	int distx = x - x_;
	int disty = y - y_;

	unsigned dist;
	if (density == 0)
		dist = 0;  // jump there directly
	else if (distx == 0)
		dist = abs(disty);
	else if (disty == 0)
		dist = abs(distx);
	else
		dist = usqrt4(distx * distx + disty * disty);

	int n = (dist * density) >> (FP + 11);
	// n = (n + FP_ROUND) >> FP;  // discard fractional part
	if (n > 1) {
		int dx = (distx << 8) / n;
		int dy = (disty << 8) / n;

		for (unsigned i = 1; i < n; i++) {
			push_sample(
				x_ + ((i * dx) >> 8),
				y_ + ((i * dy) >> 8),
				0,
				0
			);
			n_samples++;
		}
	}

	push_sample(x, y, 0, 0);
	n_samples++;

	x_cur = x;
	y_cur = y;
	return n_samples;
}

unsigned push_list(draw_list_t *p, unsigned n_items)
{
	unsigned n_samples = 0;
	while (n_items--) {
		switch (p->type) {
		case 1:
			n_samples += push_line(p->x, p->y, p->density);
			break;
		case 2:
			n_samples += push_circle(p->x, p->y, p->density);
			break;
		default:
			printf("unknown type %d\n", p->type);
		}
		p++;
	}
	return n_samples;
}
