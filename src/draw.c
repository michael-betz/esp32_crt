#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "i2s.h"
#include "draw.h"
#include "fast_sin.h"
#include "print.h"  // for printing fixed-point numbers
#include "b_font.h"

// current cursor position
static int x_cur = 0;
static int y_cur = 0;


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

static bool output_sample(int a, int b, int c, int d)
{
	bool is_clipped = false;
	if (a >= FP_MAX) {
		a = FP_MAX - 1;
		is_clipped = 1;
	} else if (a < -FP_MAX) {
		a = -FP_MAX;
		is_clipped = 1;
	}
	if (b >= FP_MAX) {
		b = FP_MAX - 1;
		is_clipped = 1;
	} else if (b < -FP_MAX) {
		b = -FP_MAX;
		is_clipped = 1;
	}
	push_sample(a + 0x800, b + 0x800, c, d);
	return is_clipped;
}

void push_circle(
	int x_a,
	int y_a,
	unsigned r_x,
	unsigned r_y,
	unsigned alpha_start,  // 0
	unsigned alpha_length,  // MAX_ANGLE
	unsigned density
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
	if (n < 3)
		n = 3;
	// printf("x: %d, y: %d, r: %d, density: %d, n: %d, INT_MAX: %x\n", x_a, y_a, r_x, density, n, INT_MAX);

	int d_alpha = (alpha_length << 8) / n;  // angular step-size

	int x, y, arg;
	for (unsigned i = 0; i <= n; i++) {
		arg = alpha_start + ((i * d_alpha) >> 8);
		x = x_a + (((get_cos(arg) >> 16) * (int)r_x) >> 15);
		y = y_a + (((get_sin(arg) >> 16) * (int)r_y) >> 15);
		// Skip seek to first point if we are already there
		if (i == 0 && x == x_cur && y == y_cur)
			continue;
		bool is_clipped = output_sample(x, y, i == 0 ? 0 : 0xFFF, 0);
		if (is_clipped)
			break;
	}
}

void push_goto(int x_a, int y_a)
{
	if (x_a == x_cur && y_a == y_cur)
		return;

	output_sample(x_a, y_a, 0, 0);
	x_cur = x_a;
	y_cur = y_a;
}

void push_line(int x_b, int y_b, unsigned density)
{
	unsigned dist;

	if (density == 0) {
		push_goto(x_b, y_b);
		return;
	}

	// Calculate the distance in x, y and hypotenuse
	int distx = x_b - x_cur;
	int disty = y_b - y_cur;

	if (distx == 0)
		dist = abs(disty);
	else if (disty == 0)
		dist = abs(distx);
	else
		dist = usqrt4(distx * distx + disty * disty);

	// Check how many points to produce
	int n = (dist * density) >> (FP + 11);

	// Don't interpolate points, just output the final point
	if (n <= 1) {
		output_sample(x_b, y_b, 0xFFF, 0);
		x_cur = x_b;
		y_cur = y_b;
		return;
	}

	int dx = (distx << 8) / n;
	int dy = (disty << 8) / n;

	for (int i = 1; i <= n; i++) {
		bool is_clipped = output_sample(
			x_cur + ((i * dx) >> 8),
			y_cur + ((i * dy) >> 8),
			0xFFF,
			0
		);
		if (is_clipped)
			break;
	}

	x_cur = x_b;
	y_cur = y_b;
}


static unsigned get_char_width(char c)
{
	if (c < 0x20)
		return 0;
	c -= 0x20;
	const uint8_t *codes = Font[(unsigned)c];
	while (1) {
		unsigned type = *codes++;
		if (type & 0x80)
			return (type & 0x7F);
		codes += 6;
	}
}

static unsigned get_str_width(char *c, unsigned scale)
{
	unsigned w = 0;
	while (*c) {
		if (*c == '\n')
			break;
		w += (get_char_width(*c) + 3) * scale / 64;
		c++;
	}
	return w;
}

void push_str(char *c, unsigned align, unsigned scale, unsigned density)
{
	unsigned x_start = x_cur;
	int w = -1;
	while (*c) {
		if (*c == '\n') {
			y_cur -= 26 * scale / 64;
			w = -1;
			c++;
			continue;
		}
		if (w == -1) {
			w = get_str_width(c, scale);
			if (align == A_RIGHT)
				x_cur = x_start - w;
			else if (align == A_CENTER)
				x_cur = x_start - w / 2;
			else
				x_cur = x_start;
		}
		push_char(*c, scale, density);
		c++;
	}
}

void push_char(char c, unsigned scale, unsigned density)
{
	unsigned x_c = x_cur, y_c = y_cur;
	// printf("push_char(%c, %d)\n", c, density);
	if (c < 0x20)
		return;
	c -= 0x20;
	const uint8_t *codes = Font[(unsigned)c];
	while (1) {
		unsigned type = *codes++;
		// printf("(%x) ", type);
		if (type & 0x80) {
			unsigned width = type & 0x7F;
			// printf("done, width: %d\n", width);
			x_cur = x_c + ((width + 3) * scale / 64);
			y_cur = y_c;
			return;
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
				push_goto(x_c + (a_x * scale / 64), y_c + (a_y * scale / 64));
				push_line(x_c + (b_x * scale / 64), y_c + (b_y * scale / 64), density);
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
				// push_goto(x_c + (a_x * scale), y_c + (a_y * scale));
				unsigned a_start = fo * MAX_ANGLE / 8;
				unsigned a_stop = (lo + 1) * MAX_ANGLE / 8;
				push_circle(
					x_c + (a_x * scale / 64),
					y_c + (a_y * scale / 64),
					(b_x * scale) / 2 / 64,
					(b_y * scale) / 2 / 64,
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
				return;
		}
	}
	return;
}

unsigned push_list(draw_list_t *p, unsigned n_items)
{
	n_samples = 0;
	while (n_items--) {
		switch (p->type) {
		case 0:
			push_goto(p->x, p->y);
			break;
		case 1:
			push_line(p->x, p->y, p->density);
			break;
		case 2:
			push_circle(x_cur, y_cur, p->x, p->y, 0, MAX_ANGLE, p->density);
			break;
		default:
			printf("unknown type %d\n", p->type);
		}
		p++;
	}
	return n_samples;
}
