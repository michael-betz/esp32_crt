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
//  2 = quadratic_bezier(x_b, y_b, x_c, y_c)
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
#define F_QBEZ 2
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

static void push_char(char c, unsigned scale, unsigned density)
{
	printf("push_char(%c, %d, %d)\n", c, scale, density);
	// Find the glyph data for the ascii letter c
	if (c < 0x20)
		return;

	unsigned glyph_index = c - 0x20;
	if (glyph_index >= current_font->n_glyphs)
		return;

	const glyph_dsc_t *glyph_dsc = &current_font->glyph_dsc[glyph_index];

	// Find the beginning and end of the glyph blob
	unsigned data_end = glyph_dsc->end_index;
	unsigned data_start = 0;
	if (glyph_index > 0)
		data_start = current_font->glyph_dsc[glyph_index - 1].end_index;
	const uint8_t *p = &current_font->glyphs[data_start];
	const uint8_t *p_end = &current_font->glyphs[data_end];

	// reset coordinate decoder state
	coordinateDecoder(NULL, NULL, NULL);

	int x = 0, y = 0;  // raw glyph coordinates (relative to glyph origin)
	int x_b = 0, y_b = 0;  // auxilary coordinates
	int x_s = 0, y_s = 0; // scaled absolute coordinates
	while (p < p_end) {
		// Upper 4 bit of the first byte indicates what to draw
		unsigned type = *p >> 4;

		if (type == F_END)
			break;

		p = coordinateDecoder(p, &x, &y);
		x_s = x_c + (x * (int)scale / 64);
		y_s = y_c + (y * (int)scale / 64);

		switch (type) {
			case F_GOTO:
				printf("goto (%d, %d)\n", x, y);
				push_goto(x_s, y_s);
				break;

			case F_LINETO:
				printf("lineto (%d, %d)\n", x, y);
				push_line(x_s, y_s, density);
				break;

			case F_QBEZ:
				p = coordinateDecoder(p, &x_b, &y_b);
				printf("qbez (), (%d, %d), (%d, %d)\n", x, y, x_b, y_b);
				push_q_bezier(
					x_s,
					y_s,
					x_c + (x_b * (int)scale / 64),
					y_c + (y_b * (int)scale / 64),
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
				printf("arc (%d, %d), %d, %d, %d, %d\n", x, y, r_x, r_y, fo, lo);
				push_circle(
					x_s,
					y_s,
					(r_x * (int)scale) / 2 / 64,
					(r_y * (int)scale) / 2 / 64,
					a_start,
					a_stop - a_start,
					density
				);
				break;

			default:
				// not implemented
				break;
		}
	}

	// Advance the cursor by the correct amount
	x_c += glyph_dsc->adv_w * (int)scale / 64;
}

void push_str(int x_a, int y_a, char *c, unsigned n, unsigned align, unsigned scale, unsigned density)
{
	y_c = y_a;
	int w_str = -1;
	while (*c && n > 0) {
		if (*c == '\n') {
			y_c -= 1000 * scale / 64;
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
