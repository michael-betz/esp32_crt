#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "i2s.h"
#include "draw.h"
#include "fast_sin.h"
#include "print.h"  // for printing fixed-point numbers
#include "b_font.h"

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

unsigned push_circle(
	uint16_t r_x,
	uint16_t r_y,
	uint16_t alpha_start,  // 0
	uint16_t alpha_length,  // MAX_ANGLE
	uint8_t density
) {
	if (r_x == 0)
		r_x = r_y;
	if (r_y == 0)
		r_y = r_x;
	if (alpha_length > MAX_ANGLE)
		alpha_length = MAX_ANGLE;
	int r_max = r_x > r_y ? r_x : r_y;
	unsigned n = (2 * r_max * density * (unsigned)(M_PI * 64) / 64) >> (FP + 11);
	n = n * alpha_length / MAX_ANGLE;
	if (n <= 0)
		return 0;
	// printf("r: %d, density: %d, n: %d\n", r >> FP, density, n);

	int d_alpha = (alpha_length << 8) / n;

	// if (alpha_length >= MAX_ANGLE)
	// 	alpha_start = random();

	for (unsigned i = 0; i <= n; i++) {
		int arg = alpha_start + ((i * d_alpha) >> 8);
		push_sample(
			x_cur + (((get_cos(arg) >> 16) * r_x) >> (FP + 11)),
			y_cur + (((get_sin(arg) >> 16) * r_y) >> (FP + 11)),
			i == 0 ? 0 : 0xFFF,
			0
		);
	}
	return n;
}

unsigned push_goto(int16_t x, int16_t y)
{
	push_sample(x, y, 0, 0);
	x_cur = x;
	y_cur = y;
	return 0;
}

unsigned push_line(int16_t x, int16_t y, uint8_t density)
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
				0xFFF,
				0
			);
			n_samples++;
		}
	}

	push_sample(x, y, dist > 0 ? 0xFFF : 0, 0);
	n_samples++;

	x_cur = x;
	y_cur = y;
	return n_samples;
}

unsigned push_char(char c, unsigned scale, unsigned density)
{
	unsigned samples = 0, x_c = x_cur, y_c = y_cur;
	// printf("push_char(%c, %d)\n", c, density);
	if (c == '\n') {
		y_cur = y_c - (26 << scale);
		x_cur = -900 << FP;
		return 0;
	} else if (c < 0x20) {
		return 0;
	}
	c -= 0x20;
	const uint8_t *codes = Font[(unsigned)c];
	while (1) {
		unsigned type = *codes++;
		// printf("(%x) ", type);
		if (type & 0x80) {
			unsigned width = type & 0x7F;
			// printf("done, width: %d\n", width);
			x_cur = x_c + ((width + 2) << scale);
			y_cur = y_c;
			return samples;
		}
		uint8_t a_x, a_y, b_x, b_y, fo, lo;
		switch (type) {
			case lin:
				// lin,XS,YS,XE,YE,FO,LO{,width|0x80}
				a_x = *codes++;  // X start of line
				a_y = *codes++;  // Y start of line
				b_x = *codes++;  // X end of line
				b_y = *codes++;  // Y end of line
				codes += 2;
				// printf("%d %d %d %d\n", a_x, a_y, b_x, b_y);
				push_goto(x_c + (a_x << scale), y_c + (a_y << scale));
				push_line(x_c + (b_x << scale), y_c + (b_y << scale), density);
				break;
			case cir:
				// cir,XC,YC,XS,YS,FO,LO{,width|0x80}
				a_x = *codes++;  // X Offset from LL corner to center of circle
				a_y = *codes++;  // Y Offset from LL corner to center of circle
				b_x = *codes++;  // Width of circle in diameter units
				b_y = *codes++;  // Height of circle in diameter units
				fo = *codes++;  // Start angle 0..7 is 0 deg .. 315 deg 0 = E, 90 = N, 180 = W, 270 = S
				lo = *codes++;  // End angle 1..14 is 45 deg .. 630 deg
				// printf("%d %d %d %d %d %d\n", a_x, a_y, b_x, b_y, fo, lo);
				// push_goto(x_c + (a_x << scale), y_c + (a_y << scale));
				x_cur = x_c + (a_x << scale);
				y_cur = y_c + (a_y << scale);
				unsigned a_start = fo * MAX_ANGLE / 8;
				unsigned a_stop = (lo + 1) * MAX_ANGLE / 8;
				push_circle(
					(b_x << scale) / 2,
					(b_y << scale) / 2,
					a_start,
					a_stop - a_start,
					density
				);
				// for (unsigned i = 0; i < 7; i++) {
				// 	printf("%d ", *codes++);
				// }
				// printf("\n");
				break;
			default:
				printf("unexpected code %d", *codes);
				return samples;
		}
	}
	return samples;
}

unsigned push_list(draw_list_t *p, unsigned n_items)
{
	unsigned n_samples = 0;
	while (n_items--) {
		switch (p->type) {
		case 0:
			n_samples += push_goto(p->x, p->y);
			break;
		case 1:
			n_samples += push_line(p->x, p->y, p->density);
			break;
		case 2:
			n_samples += push_circle(p->x, p->y, 0, MAX_ANGLE, p->density);
			break;
		default:
			printf("unknown type %d\n", p->type);
		}
		p++;
	}
	return n_samples;
}
