#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "draw.h"
#include "font_draw.h"
#include "font_data.h"
#include "fast_sin.h"


// glyph data format:

// # byte 0: `<type:4>`<flags:4>
// `type` enumerates the drawing primitive:
//  0 = goto(x, y)
//  1 = lineto(x, y)
//  2 = quadratic_bezier_off_curve(x, y)
//  3 = quadratic_bezier_on_curve(x, y)
//  4 = arc(x, y, rx:uint8, ry:uint8, fo:uint4, lo:uint4)
//  F = end of glyph
//
// `flags` indicates the coordinate encoding
// y_pos, x_pos, y_short, x_short
//
// If `x_short` is set, the x-coordinate is encoded in the next byte as an
// uint8. It describes the relative offset to the previous x-coordinate (or 0 if it's the first one).
// Its sign is given by the `x_pos` bit.
// If `x_short` is not set, the absolute x-coordinate is encoded as an int16 in the next 2 bytes
// In this case, `x_pos` is reserved for other purpose.
//
// The arc command consists of the x and y coordinates of it's center point (encoded as described above)
// then follows 3 bytes encoding x and y-radius and segment start and stop angle


#define F_GOTO 0
#define F_LINETO 1
#define F_QBEZ_ON 2
#define F_QBEZ_OFF 3
#define F_ARC 4
#define F_END 0xF

#define F_X_SHORT 1
#define F_Y_SHORT 2
#define F_X_POS 4
#define F_Y_POS 8



static const font_t *current_font = &f_arc;

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

	if (c >= current_font->n_glyphs)
		return 0;

	int w = current_font->glyph_dsc[(unsigned)c].adv_w;

	return w;
}

static int get_str_width(char *c, unsigned n, unsigned scale)
{
	int w = 0;
	if (c == NULL)
		return w;
	while (n-- > 0) {
		if (*c == '\0' || *c == '\n')
			break;
		w += (get_char_width(*c)) * scale / 64;
		c++;
	}
	return w;
}

// static void _push_char_lin(const int8_t *p, unsigned len, unsigned scale, unsigned density)
// {
// 	bool is_pen_up = true, is_clipped = false;

// 	if (len < 2)
// 		return;

// 	// first 2 bytes are the left and right limits of the glyph bounding box
// 	int l = *p++ * (int)scale / 64;
// 	int r = *p++ * (int)scale / 64;
// 	len -= 2;

// 	x_c -= l;

// 	while (len >= 2) {
// 		// Read chunks of 2 bytes
// 		int x = *p++;  // X start of line
// 		int y = *p++;  // Y start of line
// 		len -= 2;

// 		if (x == -50 && y == 0) {
// 			// pen up command, next command is a goto
// 			is_pen_up = true;
// 			continue;
// 		}
// 		if (is_pen_up) {
// 			is_clipped = push_goto(
// 				x_c + (x * (int)scale / 64),
// 				y_c - (y * (int)scale / 64)
// 			);
// 			if (!is_clipped)
// 				is_pen_up = false;
// 			continue;
// 		}
// 		is_clipped = push_line(
// 			x_c + (x * (int)scale / 64),
// 			y_c - (y * (int)scale / 64),
// 			density
// 		);
// 		if (is_clipped)
// 			is_pen_up = true;
// 	}

// 	x_c += r;
// }

// static void _push_char_arc(const int8_t *p, unsigned len, unsigned scale, unsigned density)
// {
// 	while (len > 5) {
// 		// Read chunks of 5 bytes
// 		unsigned type = *p++;  // 0 for lines, otherwise circle
// 		int a_x = *p++;  // X start of line / center of circle
// 		int a_y = *p++;  // Y start of line / center of circle
// 		int b_x = *p++;  // X end of line / x-radius
// 		int b_y = *p++;  // Y end of line / y-radius
// 		len -= 5;

// 		if (type == 0) {
// 			push_goto(
// 				x_c + (a_x * (int)scale / 64),
// 				y_c + (a_y * (int)scale / 64)
// 			);
// 			push_line(
// 				x_c + (b_x * (int)scale / 64),
// 				y_c + (b_y * (int)scale / 64),
// 				density
// 			);
// 		} else {
// 			int fo = (type >> 4) & 0xF;  // Start angle 0..7 is 0 deg .. 315 deg 0 = E, 90 = N, 180 = W, 270 = S
// 			int lo = type & 0xF;  // End angle 1..14 is 45 deg .. 630 deg
// 			unsigned a_start = fo * MAX_ANGLE / 8;
// 			unsigned a_stop = (lo + 1) * MAX_ANGLE / 8;
// 			push_circle(
// 				x_c + (a_x * (int)scale / 64),
// 				y_c + (a_y * (int)scale / 64),
// 				(b_x * (int)scale) / 2 / 64,
// 				(b_y * (int)scale) / 2 / 64,
// 				a_start,
// 				a_stop - a_start,
// 				density
// 			);
// 		}
// 	}
// 	if (len >= 1) {
// 		// last byte is the width, advance the cursor
// 		x_c += (*p + 3) * (int)scale / 64;
// 	}
// }





static void push_char(char c, unsigned scale, unsigned density)
{
	if (c < 0x20)
		return;
	c -= 0x20;

	if (c >= current_font->n_glyphs)
		return;

	const glyph_dsc_t *gdsc = &current_font->glyph_dsc[(unsigned)c];

	const uint8_t *p = &current_font->glyphs[gdsc->start_index];

	// p points to a flag - byte
	// printf("%d --> %d, %02x\n", c, gdsc->start_index, *p);

	int x = 0, y = 0;
	while (1) {
		unsigned flags = *p & 0xF;
		unsigned type = *p >> 4;
		p++;

		if (type == F_END)
			break;

		if (flags & F_X_SHORT) {
			if (flags & F_X_POS)
				x += *p;
			else
				x -= *p;
			p++;
		} else {
			int16_t tmp = (*p++) << 8;
			tmp |= *p++;
			x = tmp;
		}

		if (flags & F_Y_SHORT) {
			if (flags & F_Y_POS)
				y += *p;
			else
				y -= *p;
			p++;
		} else {
			int16_t tmp = (*p++) << 8;
			tmp |= *p++;
			y = tmp;
		}

		switch (type) {
			case F_GOTO:
				push_goto(
					x_c + (x * (int)scale / 64),
					y_c + (y * (int)scale / 64)
				);
				break;

			case F_LINETO:
				push_line(
					x_c + (x * (int)scale / 64),
					y_c + (y * (int)scale / 64),
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
				push_circle(
					x_c + (x * (int)scale / 64),
					y_c + (y * (int)scale / 64),
					(r_x * (int)scale) / 2 / 64,
					(r_y * (int)scale) / 2 / 64,
					a_start,
					a_stop - a_start,
					density
				);
				break;

			default:
			// case T_QBEZ_ON:
			// case T_QBEZ_OFF:
				// not implemented
				break;
		}
	}
	x_c += (gdsc->adv_w + 3) * (int)scale / 64;
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
			w_str = get_str_width(c, n, scale);
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
