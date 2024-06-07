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
		w += (get_char_width(*c)) * scale / current_font->units_per_em;
		c++;
	}
	return w;
}

static void push_char(char c, unsigned scale, unsigned density)
{
	// printf("push_char(%c, %d, %d)\n", c, scale, density);
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
	unsigned len = &current_font->glyphs[data_end] - p;

	draw_blob(p, len, x_c, y_c, scale, current_font->units_per_em, density);

	// Advance the cursor by the correct amount
	x_c += glyph_dsc->adv_w * (int)scale / current_font->units_per_em;
}

void push_str(int x_a, int y_a, char *c, unsigned n, unsigned align, unsigned scale, unsigned density)
{
	y_c = y_a;
	int w_str = -1;
	while (*c && n > 0) {
		if (*c == '\n') {
			y_c -= scale;
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
