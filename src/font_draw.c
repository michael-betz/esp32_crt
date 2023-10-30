#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "draw.h"
#include "font_draw.h"
#include "font_data.h"
#include "fast_sin.h"

// static font_t *current_font = &f_arc;
static font_t *current_font = &f_romancs;

static unsigned get_char_width(char c)
{
	if (c < 0x20)
		return 0;
	c -= 0x20;

	// get stop index (last byte is always char width)
	unsigned b = current_font->inds[(unsigned)c];

	return current_font->glyphs[b - 1];
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

static int _push_char_lin(const int8_t *p, unsigned len, int x_c, int y_c, unsigned scale, unsigned density)
{
	bool is_pen_up = true;

	while (len > 2) {
		// Read chunks of 2 bytes
		int x = *p++;  // X start of line
		int y = *p++;  // Y start of line
		len -= 2;

		if (is_pen_up) {
			push_goto(
				x_c + (x * (int)scale / 64),
				y_c - (y * (int)scale / 64)
			);
			is_pen_up = false;
			continue;
		}
		if (x == -50 && y == 0) {
			// pen up command, next command is a goto
			is_pen_up = true;
			continue;
		}
		push_line(
			x_c + (x * (int)scale / 64),
			y_c - (y * (int)scale / 64),
			density
		);
	}
	if (len >= 1) {
		// last byte is the width, return it
		return *p;
	}
	return 0;
}

static int _push_char_arc(const int8_t *p, unsigned len, int x_c, int y_c, unsigned scale, unsigned density)
{
	while (len > 5) {
		// Read chunks of 5 bytes
		unsigned type = *p++;  // 0 for lines, otherwise circle
		int a_x = *p++;  // X start of line / center of circle
		int a_y = *p++;  // Y start of line / center of circle
		int b_x = *p++;  // X end of line / x-radius
		int b_y = *p++;  // Y end of line / y-radius
		len -= 5;

		if (type == 0) {
			push_goto(
				x_c + (a_x * (int)scale / 64),
				y_c + (a_y * (int)scale / 64)
			);
			push_line(
				x_c + (b_x * (int)scale / 64),
				y_c + (b_y * (int)scale / 64),
				density
			);
		} else {
			int fo = (type >> 4) & 0xF;  // Start angle 0..7 is 0 deg .. 315 deg 0 = E, 90 = N, 180 = W, 270 = S
			int lo = type & 0xF;  // End angle 1..14 is 45 deg .. 630 deg
			unsigned a_start = fo * MAX_ANGLE / 8;
			unsigned a_stop = (lo + 1) * MAX_ANGLE / 8;
			push_circle(
				x_c + (a_x * (int)scale / 64),
				y_c + (a_y * (int)scale / 64),
				(b_x * (int)scale) / 2 / 64,
				(b_y * (int)scale) / 2 / 64,
				a_start,
				a_stop - a_start,
				density
			);
		}
	}
	if (len >= 1) {
		// last byte is the width, return it
		return *p;
	}
	return 0;
}

int push_char(int x_c, int y_c, char c, unsigned scale, unsigned density)
{
	// printf("push_char(%c, %d, %d)\n", c, scale, density);
	if (c < 0x20)
		return 0;
	c -= 0x20;

	// get start and stop indices
	unsigned a = 0, b = 0;
    if (c > 0)
		a = current_font->inds[c - 1];
	b = current_font->inds[(unsigned)c];

	const int8_t *p = &current_font->glyphs[a];
	switch (current_font->font_type) {
	case FONT_TYPE_LIN:
		return _push_char_lin(p, b - a, x_c, y_c, scale, density);

	case FONT_TYPE_ARC:
		return _push_char_arc(p, b - a, x_c, y_c, scale, density);
	}
	return 0;
}

void push_str(int x_a, int y_a, char *c, unsigned n, unsigned align, unsigned scale, unsigned density)
{
	int w_str = -1;
	int x_c = 0;
	while (*c && n > 0) {
		if (*c == '\n') {
			y_a -= 26 * scale / 64;
			w_str = -1;
			c++;
			n--;
			continue;
		}
		if (w_str == -1) {
			w_str = get_str_width(c, scale);
			if (align == A_RIGHT)
				x_c = x_a - w_str;
			else if (align == A_CENTER)
				x_c = x_a - w_str / 2;
			else
				x_c = x_a;
		}
		int w_char = push_char(x_c, y_a, *c, scale, density);
		x_c += (w_char + 3) * scale / 64;
		n--;
		c++;
	}
}
