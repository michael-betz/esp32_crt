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

#define DWELL_TIME 4  // how many extra samples at the end-point of a shape to wait for the beam

#define BLANK_OFF_TIME 10  // How long it takes to enable / disable the beam [n_samples]
#define BLANK_DENSITY 10  // Density for blanked move [density]
#define BLANK_MIN_DIST 10  // Blank only for distances larger than that


// current beam position. Use push_goto() to modify these values.
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
	bool is_clipped = false;

	// printf("(%4d, %4d), %3x\n", x, y, blank);
	if (x > C_MAX) {
		x = C_MAX;
		is_clipped = true;
	}
	if (x < -C_MAX) {
		x = -C_MAX;
		is_clipped = true;
	}
	if (y > C_MAX) {
		y = C_MAX;
		is_clipped = true;
	}
	if (y < -C_MAX) {
		y = -C_MAX;
		is_clipped = true;
	}

	push_sample(x + 0x800, y + 0x800, beam_on ? 0 : 0xFFF, 0);
	return is_clipped;
}

// draw a quadratic Bezier curve. The 3 control points are:
// the current beam position, (x1, y1), (x2, y2)
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

// draw a cubic Bezier curve. The 4 control points are:
// the current beam position, (x1, y1), (x2, y2), (x3, y3)
// We'll do it in 20.12 fixed-point (precision is controlled by MAX_ANGLE)
void push_c_bezier(int x1, int y1, int x2, int y2, int x3, int y3, int density)
{
	unsigned dist = get_dist(x3, y3);

	// How many points to interpolate based on linear distance
	int n = (dist * density) >> 11;

	if (n <= 2) {
		// Output 2 points (the beginning and end-points)
	} else if (n <= 3) {
		// output 3 points (the middle one is interpolated)
		// xs = 0.5**3 * P0 + 3 * 0.5**2 * 0.5 * P1 + 3 * 0.5 * 0.5**2 * P2 + 0.5**3 * P3
		// xs = 0.125 * P0 + 0.375 * P1 + 0.375 * P2 + 0.125 * P3
		// xs = P0 / 8 + 3 * P1 / 8 + 3 * P2 / 8 + P3 / 8
		// xs = (P0 + 3 * (P1 + P2) + P3) / 8
		output_sample(
			(x_last + 3 * (x1 + x2) + x3) / 8,
			(y_last + 3 * (y1 + y2) + y3) / 8,
			true,
			0
		);
	} else {
		// output n points, need to interpolate n - 2 points.
		int d_t = MAX_ANGLE / n;  // t step-size (n steps from 0 to MAX_ANGLE)

		for (unsigned i = 1; i <= (n - 1); i++) {
			int t = i * d_t;

			// xs = (1 - t)**3 * P0 + 3 * (1 - t)**2 * t * P1 + 3 * (1 - t) * t**2 * P2 + t**3 * P3
			// xs =          A * P0 +          3 * B * t * P1 + 3 * (1 - t) *    C * P2 +    D * P3
			long int B = (MAX_ANGLE - t) * (MAX_ANGLE - t) / MAX_ANGLE;
			int A = B * (MAX_ANGLE - t) / MAX_ANGLE;
			long int C = t * t / MAX_ANGLE;
			int D = C * t / MAX_ANGLE;

			int xs = A * x_last + (3 * B * t * x1) / MAX_ANGLE + (3 * (MAX_ANGLE - t) * C * x2) / MAX_ANGLE + D * x3;
			int ys = A * y_last + (3 * B * t * y1) / MAX_ANGLE + (3 * (MAX_ANGLE - t) * C * y2) / MAX_ANGLE + D * y3;

			// Round from fixed point back to integer
			xs = (xs + MAX_ANGLE / 2) / MAX_ANGLE;
			ys = (ys + MAX_ANGLE / 2) / MAX_ANGLE;

			output_sample(xs, ys, true, 0);
		}
	}

	output_sample(x3, y3, true, 0);
	x_last = x3;
	y_last = y3;
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

	// output last sample a few more time to make sure beam had a chance to close the circle
	if (!is_clipped)
		for (unsigned i=0; i<DWELL_TIME; i++)
			output_sample(x_last, y_last, true, 0);

	return is_clipped;
}

bool push_goto(int x_a, int y_a)
{
	bool is_clipped = false;

	if (x_a == x_last && y_a == y_last)
		return false;

	unsigned dist = get_dist(x_a, y_a);
	unsigned n = (dist * BLANK_DENSITY) >> 11;

	// Wait at current location until the beam has turned off
	if (dist >= BLANK_MIN_DIST)
		for (unsigned i=0; i<BLANK_OFF_TIME; i++)
			output_sample(x_last, y_last, false, 0);

	// Move the beam while blanked. Interpolate N points for the move.
	for (unsigned i=0; i<=n; i++) {
		is_clipped = output_sample(x_a, y_a, false, 0);
		if (is_clipped)
			break;
	}
	x_last = x_a;
	y_last = y_a;

	// Wait for beam to reach new location
	if (dist >= BLANK_MIN_DIST && !is_clipped)
		for (unsigned i=0; i<BLANK_OFF_TIME; i++)
			output_sample(x_a, y_a, false, 0);

	// Wait at new location until the beam is back on
	if (dist >= BLANK_MIN_DIST && !is_clipped)
		for (unsigned i=0; i<DWELL_TIME; i++)
			output_sample(x_a, y_a, true, 0);

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
	int x = 0, y = 0;

	for (int i = 1; i <= n; i++) {
		x = x_last + ((i * dx) >> 8);
		y = y_last + ((i * dy) >> 8);
		is_clipped = output_sample(x, y, true, 0);
		if (is_clipped)
			break;
	}
	x_last = x_b;
	y_last = y_b;

	// wait for beam to reach end-point
	if (!is_clipped)
		for (unsigned i=0; i<DWELL_TIME; i++)
			output_sample(x_last, y_last, true, 0);

	return is_clipped;
}

// Compress 16 bit signed coordinate pairs and encode them in a byte stream
// Very similar to how true-type fonts encode their coordinate values.
static const uint8_t *coordinateDecoder(const uint8_t *p, int *x_out, int *y_out)
{
		// Holds internal state
		static int x = 0, y = 0;

		if (p == NULL) {
			// reset internal state
			x = 0;
			y = 0;
			return p;
		}

		// Consume 1 byte encoding the flags
		unsigned flags = *p++ & 0xF;

		if (flags & F_X_SHORT) {
			// Short number format, consume one byte
			// F_X_POS indicates if the byte is positive or negative
			if (flags & F_X_POS)
				x += *p;
			else
				x -= *p;
			p++;
		} else {
			// Long number format. Consume 2 bytes if F_X_POS is set.
			// Otherwise consume 0 bytes and don't change the value
			if (flags & F_X_POS) {
				int16_t tmp = (*p++) << 8;
				tmp |= *p++;
				x = tmp;
			}
		}

		if (flags & F_Y_SHORT) {
			if (flags & F_Y_POS)
				y += *p;
			else
				y -= *p;
			p++;
		} else {
			if (flags & F_Y_POS) {
				int16_t tmp = (*p++) << 8;
				tmp |= *p++;
				y = tmp;
			}
		}

		*x_out = x;
		*y_out = y;

		return p;
}

// Run through a serialized draw-list. Used for font-glyphs too.
void draw_blob(const uint8_t *p, unsigned n_bytes, int x_center, int y_center, int scale, int scale_div, int density)
{
	const uint8_t *p_end = p + n_bytes;

	// reset coordinate decoder state
	coordinateDecoder(NULL, NULL, NULL);

	int x = 0, y = 0;  // raw coordinates
	int x_b = 0, y_b = 0, x_c = 0, y_c = 0;  // auxiliary coordinates
	int x_s = 0, y_s = 0; // scaled absolute coordinates

	while (p < p_end) {
		// Upper 4 bit of the first byte indicates what to draw
		unsigned type = *p >> 4;

		if (type == F_END)
			break;

		p = coordinateDecoder(p, &x, &y);
		x_s = x_center + (x * (int)scale / scale_div);
		y_s = y_center + (y * (int)scale / scale_div);

		switch (type) {
			case F_GOTO:
				// printf("goto (%d, %d)\n", x, y);
				push_goto(x_s, y_s);
				break;

			case F_LINETO:
				// printf("lineto (%d, %d)\n", x, y);
				push_line(x_s, y_s, density);
				break;

			case F_QBEZ:
				p = coordinateDecoder(p, &x_b, &y_b);
				// printf("qbez (), (%d, %d), (%d, %d)\n", x, y, x_b, y_b);
				push_q_bezier(
					x_s,
					y_s,
					x_center + (x_b * (int)scale / scale_div),
					y_center + (y_b * (int)scale / scale_div),
					density
				);
				break;

			case F_CBEZ:
				p = coordinateDecoder(p, &x_b, &y_b);
				p = coordinateDecoder(p, &x_c, &y_c);
				// printf("cbez (), (%d, %d), (%d, %d), (%d, %d)\n", x_s, y_s, x_b, y_b, x_c, y_c);
				push_c_bezier(
					x_s,
					y_s,
					x_center + (x_b * (int)scale / scale_div),
					y_center + (y_b * (int)scale / scale_div),
					x_center + (x_c * (int)scale / scale_div),
					y_center + (y_c * (int)scale / scale_div),
					density
				);
				break;

			case F_ARC:
				unsigned r_x = *p++;
				unsigned r_y = *p++;
				int fo = (*p >> 4) & 0xF;  // Start angle 0..7 is 0 deg .. 315 deg 0 = E, 90 = N, 180 = W, 270 = S
				int lo = *p & 0xF;  // End angle 1..14 is 45 deg .. 630 deg
				p++;
				unsigned a_start = fo * MAX_ANGLE / 8;
				unsigned a_stop = (lo + 1) * MAX_ANGLE / 8;
				// printf("arc (%d, %d), %d, %d, %d, %d\n", x, y, r_x, r_y, fo, lo);
				push_circle(
					x_s,
					y_s,
					(r_x * (int)scale) / 2 / scale_div,
					(r_y * (int)scale) / 2 / scale_div,
					a_start,
					a_stop - a_start,
					density
				);
				break;

			default:
				// not implemented
				printf("HUH! %x\n", type);
				return;
		}
	}
}
