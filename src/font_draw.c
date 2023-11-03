#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "draw.h"
#include "font_draw.h"
#include "font_data.h"
#include "fast_sin.h"

static font_t *current_font = &f_arc;

// glyph cursor
static int x_c = 0, y_c = 0;

void set_font(unsigned index)
{
	if (index < N_FONTS)
		current_font = f_all[index];
}

static int get_char_width(char c)
{
	if (c < 0x20)
		return 0;
	c -= 0x20;

	unsigned a = 0, b = 0;
	if (c > 0)
		a = current_font->inds[(unsigned)c - 1];
	b = current_font->inds[(unsigned)c];

	switch (current_font->font_type) {
	case FONT_TYPE_LIN:
		// first 2 bytes are the left and right limits of the glyph bounding box
		return current_font->glyphs[a + 1] - current_font->glyphs[a];

	case FONT_TYPE_ARC:
		// Last byte is width
		return current_font->glyphs[b - 1] + 3;
	}
	return 0;
}

static int get_str_width(char *c, unsigned scale)
{
	int w = 0;
	while (*c) {
		if (*c == '\n')
			break;
		w += (get_char_width(*c)) * scale / 64;
		c++;
	}
	return w;
}

static void _push_char_lin(const int8_t *p, unsigned len, unsigned scale, unsigned density)
{
	bool is_pen_up = true, is_clipped = false;

	if (len < 2)
		return;

	// first 2 bytes are the left and right limits of the glyph bounding box
	int l = *p++ * (int)scale / 64;
	int r = *p++ * (int)scale / 64;
	len -= 2;

	x_c -= l;

	while (len >= 2) {
		// Read chunks of 2 bytes
		int x = *p++;  // X start of line
		int y = *p++;  // Y start of line
		len -= 2;

		if (x == -50 && y == 0) {
			// pen up command, next command is a goto
			is_pen_up = true;
			continue;
		}
		if (is_pen_up) {
			is_clipped = push_goto(
				x_c + (x * (int)scale / 64),
				y_c - (y * (int)scale / 64)
			);
			if (!is_clipped)
				is_pen_up = false;
			continue;
		}
		is_clipped = push_line(
			x_c + (x * (int)scale / 64),
			y_c - (y * (int)scale / 64),
			density
		);
		if (is_clipped)
			is_pen_up = true;
	}

	x_c += r;
}

static void _push_char_arc(const int8_t *p, unsigned len, unsigned scale, unsigned density)
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
		// last byte is the width, advance the cursor
		x_c += (*p + 3) * (int)scale / 64;
	}
}

static void push_char(char c, unsigned scale, unsigned density)
{
	// printf("push_char(%c, %d, %d) (%d, %d)\n", c, scale, density, x_c, y_c);
	if (c < 0x20)
		return;
	c -= 0x20;

	// get start and stop indices
	unsigned a = 0, b = 0;
	if (c > 0)
		a = current_font->inds[c - 1];
	b = current_font->inds[(unsigned)c];

	const int8_t *p = &current_font->glyphs[a];
	switch (current_font->font_type) {
	case FONT_TYPE_LIN:
		_push_char_lin(p, b - a, scale, density);
		break;

	case FONT_TYPE_ARC:
		_push_char_arc(p, b - a, scale, density);
		break;
	}
}

void push_str(int x_a, int y_a, char *c, unsigned n, unsigned align, unsigned scale, unsigned density)
{
	y_c = y_a;
	int w_str = -1;
	while (*c && n > 0) {
		if (*c == '\n') {
			y_c -= 26 * scale / 64;
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
		push_char(*c, scale, density);
		n--;
		c++;
	}
}
