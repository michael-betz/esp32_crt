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


#define BLANK_OFF_TIME 5  // How long it takes to disable the beam [n_samples]
#define BLANK_ON_TIME 12  // How long it takes to enable the beam [n_samples]
#define BLANK_DENSITY 1  // Density for blanked move [density]
#define BLANK_MIN_DIST 1  // Blank only for distances larger than that


// current pen position. Use push_goto() to modify.
static int x_last = 0;
static int y_last = 0;


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

// return the distance from current beam position to (x, y)
static unsigned get_dist(int x, int y)
{
	// Calculate the distance in x, y and hypotenuse
	int distx = x - x_last;
	int disty = y - y_last;
	unsigned dist = 0;

	if (distx == 0)
		dist = abs(disty);
	else if (disty == 0)
		dist = abs(distx);
	else
		dist = usqrt4(distx * distx + disty * disty);

	return dist;
}

bool output_sample(int x, int y, bool beam_on, int focus)
{
	// printf("(%4d, %4d), %3x\n", x, y, blank);
	if (x >= C_MAX || x < -C_MAX || y >= C_MAX || y < -C_MAX)
		return true;

	// Only output sample if it is not clipped
	push_sample(x + 0x800, y + 0x800, beam_on ? 0 : 0xFFF, 0);
	return false;
}

// draw a quadratic Bezier curve. The 3 control points are:
// the current pen position, (x1, y1), (x2, y2)
// We'll do it in 20.12 fixed-point (precision is controlled by MAX_ANGLE)
void push_q_bezier(int x1, int y1, int x2, int y2, int density)
{
	unsigned dist = get_dist(x2, y2);

	// How many points to interpolate based on linear distance
	int n = (dist * density) >> 11;

	if (n <= 2) {
		// Output 2 points (the beginning and end-points)
	} else if (n <= 3) {
		// output 3 points (the middle one is interpolated)
		output_sample(
			(x_last + x2) / 4 + x1 / 2,
			(y_last + y2) / 4 + y1 / 2,
			true,
			0
		);
	} else {
		// output n points, need to interpolate n - 2 points.
		int d_t = MAX_ANGLE / n;  // t step-size (n steps from 0 to MAX_ANGLE)

		for (unsigned i = 1; i <= (n - 1); i++) {
			int t = i * d_t;

			// xs = (1 - t)**2 * pt0[0] + 2 * (1 - t) * t * pt1[0] + t**2 * pt2[0]
			// xs =          A * pt0[0] +               B * pt1[0] +    C * pt2[0]
			int A = (MAX_ANGLE - t) * (MAX_ANGLE - t) / MAX_ANGLE;
			int B = 2 * (MAX_ANGLE - t) * t / MAX_ANGLE;
			int C = t * t / MAX_ANGLE;

			int xs = (A * x_last + B * x1 + C * x2 + MAX_ANGLE / 2) / MAX_ANGLE;
			int ys = (A * y_last + B * y1 + C * y2 + MAX_ANGLE / 2) / MAX_ANGLE;
			output_sample(xs, ys, true, 0);
		}
	}

	output_sample(x2, y2, true, 0);
	x_last = x2;
	y_last = y2;
}

bool push_circle(
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

	int x = 0, y = 0, arg;
	bool is_clipped = false;
	for (unsigned i = 0; i <= n; i++) {
		arg = alpha_start + ((i * d_alpha) >> 8);
		x = x_a + (((get_cos(arg) >> 16) * (int)r_x) >> 15);
		y = y_a + (((get_sin(arg) >> 16) * (int)r_y) >> 15);

		// Seek to first point and draw the reset
		if (i == 0)
			push_goto(x, y);

		is_clipped = output_sample(x, y, true, 0);

		if (is_clipped)
			break;
	}
	x_last = x;
	y_last = y;
	return is_clipped;
}

bool push_goto(int x_a, int y_a)
{
	bool is_clipped = false;

	if (x_a == x_last && y_a == y_last)
		return is_clipped;

	unsigned dist = get_dist(x_a, y_a);
	unsigned n = (dist * BLANK_DENSITY) >> 11;

	// Wait at current location until the beam has turned off
	if (dist >= BLANK_MIN_DIST)
		for (unsigned i=0; i<BLANK_OFF_TIME; i++)
			output_sample(x_last, y_last, false, 0);

	// Move the beam while blanked. Interpolate N points for the move.
	for (unsigned i=0; i<=n; i++)
		is_clipped = output_sample(x_a, y_a, false, 0);
	x_last = x_a;
	y_last = y_a;

	// Wait at new location until the beam is back on
	if (dist >= BLANK_MIN_DIST)
		for (unsigned i=0; i<BLANK_ON_TIME; i++)
			output_sample(x_last, y_last, false, 0);

	return is_clipped;
}

bool push_line(int x_b, int y_b, unsigned density)
{
	if (density == 0)
		return push_goto(x_b, y_b);

	// Check how many points to produce
	unsigned dist = get_dist(x_b, y_b);
	int n = (dist * density) >> 11;

	// printf("push_line(%d, %d), n: %d\n", x_b, y_b, n);

	// Don't interpolate points, just output the final point
	if (n <= 1) {
		x_last = x_b;
		y_last = y_b;
		return output_sample(x_b, y_b, true, 0);
	}

	int dx = ((x_b - x_last) << 8) / n;
	int dy = ((y_b - y_last) << 8) / n;
	bool is_clipped = false;

	for (int i = 1; i <= n; i++) {
		is_clipped = output_sample(
			x_last + ((i * dx) >> 8),
			y_last + ((i * dy) >> 8),
			true,
			0
		);
		if (is_clipped)
			break;
	}
	x_last = x_b;
	y_last = y_b;
	return is_clipped;
}

// void push_glyph(uint8_t *p, unsigned n_bytes_max, int x_o, int y_o, unsigned scale)

void push_list(uint8_t *p, unsigned n_bytes_max)
{
	unsigned n = 0, n_total = 0;  // bytes read / iteration, / total
	while (n_total < n_bytes_max) {
		uint8_t type = *p;

		if (type == T_LINE) {
			line_t *tmp = (line_t *)p;
			push_line(tmp->x_b, tmp->y_b, tmp->density * DENSITY_MULTIPLIER);
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
				push_line(x, y, tmp->density * DENSITY_MULTIPLIER);
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
				tmp->density * DENSITY_MULTIPLIER
			);
			n = sizeof(circle_t);
		} else if ((type & 0xFC) == T_STRING) {
			string_t *tmp = (string_t *)p;
			set_font(tmp->font);
			push_str(
				tmp->x,
				tmp->y,
				tmp->c,
				tmp->len,
				type & 0x03,
				tmp->scale,
				tmp->density * DENSITY_MULTIPLIER
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
