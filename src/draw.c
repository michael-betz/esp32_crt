#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include "i2s.h"
#include "draw.h"
#include "font_draw.h"
#include "fast_sin.h"
#include "print.h"  // for printing fixed-point numbers

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
	// printf("(%4d, %4d), %3x\n", a, b, c);
	bool is_clipped = false;
	if (a >= C_MAX) {
		a = C_MAX - 1;
		is_clipped = 1;
	} else if (a < -C_MAX) {
		a = -C_MAX;
		is_clipped = 1;
	}
	if (b >= C_MAX) {
		b = C_MAX - 1;
		is_clipped = 1;
	} else if (b < -C_MAX) {
		b = -C_MAX;
		is_clipped = 1;
	}
	push_sample(a + 0x800, b + 0x800, c, d);
	return is_clipped;
}

// last outputted sample, used for drawing polygons, don't touch!
static int x_last = 0;
static int y_last = 0;

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
	unsigned n = (2 * r_max * density * (unsigned)(M_PI * 64) / 64) >> 11;
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
		if (i == 0 && x == x_last && y == y_last)
			continue;
		bool is_clipped = output_sample(x, y, i == 0 ? 0 : 0xFFF, 0);
		if (is_clipped)
			break;
	}
	x_last = x;
	y_last = y;
}

void push_goto(int x_a, int y_a)
{
	if (x_a == x_last && y_a == y_last) {
		// printf("goto(%d, %d)*\n", x_a, y_a);
		return;
	}
	// printf("goto(%d, %d)\n", x_a, y_a);

	output_sample(x_a, y_a, 0, 0);
	x_last = x_a;
	y_last = y_a;
}

void push_line(int x_b, int y_b, unsigned density)
{
	unsigned dist;

	if (density == 0) {
		push_goto(x_b, y_b);
		return;
	}

	// Calculate the distance in x, y and hypotenuse
	int distx = x_b - x_last;
	int disty = y_b - y_last;

	if (distx == 0)
		dist = abs(disty);
	else if (disty == 0)
		dist = abs(distx);
	else
		dist = usqrt4(distx * distx + disty * disty);

	// Check how many points to produce
	int n = (dist * density) >> 11;

	// printf("push_line(%d, %d), n: %d\n", x_b, y_b, n);

	// Don't interpolate points, just output the final point
	if (n <= 1) {
		output_sample(x_b, y_b, 0xFFF, 0);
		x_last = x_b;
		y_last = y_b;
		return;
	}

	int dx = (distx << 8) / n;
	int dy = (disty << 8) / n;

	for (int i = 1; i <= n; i++) {
		bool is_clipped = output_sample(
			x_last + ((i * dx) >> 8),
			y_last + ((i * dy) >> 8),
			0xFFF,
			0
		);
		if (is_clipped)
			break;
	}

	x_last = x_b;
	y_last = y_b;
}

// void push_glyph(uint8_t *p, unsigned n_bytes_max, int x_o, int y_o, unsigned scale)

void push_list(uint8_t *p, unsigned n_bytes_max)
{
	unsigned n = 0, n_total = 0;  // bytes read / iteration, / total
	while (n_total < n_bytes_max) {
		uint8_t type = *p;

		if (type == T_LINE) {
			line_t *tmp = (line_t *)p;
			push_line(tmp->x_b, tmp->y_b, tmp->density);
			n = sizeof(line_t);
		} else if (type == T_POLY) {
			poly_t *tmp = (poly_t *)p;
			unsigned j = tmp->len * 2;
			bool pen_up = false;
			for (unsigned i = 0; i < j; i += 2) {
				int x = tmp->pts[i];
				int y = tmp->pts[i + 1];
				// Most neg. coord means push a goto next
				if (x == -32768 && y == -32768) {
					pen_up = true;
					continue;
				}
				if (pen_up) {
					push_goto(x, y);
					pen_up = false;
					continue;
				}
				push_line(x, y, tmp->density);
			}
			n = sizeof(poly_t) + j * sizeof(int16_t);
		} else if (type == T_CIRCLE) {
			circle_t *tmp = (circle_t *)p;
			push_circle(
				tmp->x,
				tmp->y,
				tmp->r_x,
				tmp->r_y,
				(tmp->a_start << 4) | (tmp->a_start >> 4),
				(tmp->a_length << 4) | (tmp->a_length >> 4),
				tmp->density
			);
			n = sizeof(circle_t);
		} else if ((type & 0xFC) == T_STRING) {
			string_t *tmp = (string_t *)p;
			push_str(
				tmp->x,
				tmp->y,
				tmp->c,
				tmp->len,
				type & 0x03,
				tmp->scale,
				tmp->density
			);
			n = sizeof(string_t) + tmp->len * sizeof(char);
		} else if (type == T_END){
			return;
		} else {
			printf("unknown type %02x\n", type);
			return;
		}
		p += n;
		n_total += n;
	}
	printf("No end marker!\n");
}
