#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "draw.h"
#include "font_draw.h"
#include "fonts/font_data.h"
#include "fast_sin.h"

static const font_t *current_font = &f_arc;

// glyph cursor
static int x_c = 0, y_c = 0;

// decodes one UTF8 character at a time, keeping internal state
// returns a valid unicode character once complete or 0
static unsigned utf8_dec(char c)
{
	// Unicode return value                   UTF-8 encoded chars
	// 00000000 00000000 00000000 0xxxxxxx -> 0xxxxxxx
	// 00000000 00000000 00000yyy yyxxxxxx -> 110yyyyy 10xxxxxx
	// 00000000 00000000 zzzzyyyy yyxxxxxx -> 1110zzzz 10yyyyyy 10xxxxxx
	// 00000000 000wwwzz zzzzyyyy yyxxxxxx -> 11110www 10zzzzzz 10yyyyyy 10xxxxxx
    static unsigned readN=0, result=0;

    if ((c & 0x80) == 0) {
        // 1 byte character, nothing to decode
        readN = 0;  // reset state
        return c;
    }

    if (readN == 0) {
        result = 0;

        // first byte of several, initialize N bytes decode
        if ((c & 0xE0) == 0xC0) {
            readN = 1;  // 1 more byte to decode
            result |= (c & 0x1F) << 6;
            return 0;
        } else if ((c & 0xF0) == 0xE0) {
            readN = 2;
            result |= (c & 0x0F) << 12;
            return 0;
        } else if ((c & 0xF8) == 0xF0) {
            readN = 3;
            result |= (c & 0x07) << 18;
            return 0;
        } else {  // shouldn't happen?
            return 0;
        }
    }

    switch (readN) {
        case 1:
            result |= c & 0x3F;
            readN = 0;
            return result;

        case 2:
            result |= (c & 0x3F) << 6;
            break;

        case 3:
            result |= (c & 0x3F) << 12;
            break;

        default:
            readN = 1;
    }
    readN--;

    return 0;
}

static int binary_search(unsigned target, const unsigned *arr, int length) {
    int left = 0;
    int right = length - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        // Check if target is present at mid
        if (arr[mid] == target) {
            return mid;
        }

        // If target greater, ignore left half
        if (arr[mid] < target) {
            left = mid + 1;
        }
        // If target is smaller, ignore right half
        else {
            right = mid - 1;
        }
    }

    // Target is not present in array
    return -1;
}

// returns a pointer to the glyph-description entry for the unicode character dc
// or NULL, if the character can't be found
static const glyph_dsc_t *find_glyph_dsc(unsigned dc)
{
	int glyph_index = -1;

	// check if the character is in the ascii map
	if (dc >= current_font->map_start && (dc - current_font->map_start) < current_font->map_n) {
		glyph_index = dc - current_font->map_start;
	} else if (current_font->map_unicode_table != NULL) {
		// otherwise binary search in map_unicode_table
		glyph_index = binary_search(
			dc,
			current_font->map_unicode_table,
			current_font->n_glyphs - current_font->map_n
		);
		if (glyph_index > -1)
			glyph_index += current_font->map_n;
	}

	if (glyph_index >= 0 && glyph_index < current_font->n_glyphs)
		return &current_font->glyph_dsc[glyph_index];

	return NULL;
}

void set_font(unsigned index)
{
	if (index < N_FONTS)
		current_font = f_all[index];
}

static int get_char_width(unsigned dc, unsigned scale)
{
	const glyph_dsc_t *glyph_dsc = find_glyph_dsc(dc);
	if (glyph_dsc == NULL)
		return 0;

	return glyph_dsc->adv_w * scale / current_font->units_per_em;
}

static int get_str_width(char *c, unsigned n, unsigned scale)
{
	int w = 0;
	if (c == NULL)
		return w;

	while (*c && n > 0) {
        unsigned dc = utf8_dec(*c++);
        n--;
        if (dc == 0)
            continue;

		if (dc == '\n')
			break;

		w += (get_char_width(dc, scale));
	}
	utf8_dec('\0');  // reset internal state
	return w;
}

static void push_char(unsigned dc, unsigned scale, unsigned density)
{

	// Find the glyph data for the unicode letter dc
	const glyph_dsc_t *glyph_dsc = find_glyph_dsc(dc);
	if (glyph_dsc == NULL)
		return;

	// Find the beginning and length of the glyph blob
	unsigned data_end = glyph_dsc->end_index;
	unsigned data_start = 0;
	if (glyph_dsc > current_font->glyph_dsc)
		data_start = (glyph_dsc - 1)->end_index;
	const uint8_t *p = &current_font->glyphs[data_start];
	unsigned len = &current_font->glyphs[data_end] - p;

	// Draw the shapes of the glyph
	draw_blob(p, len, x_c, y_c, scale, current_font->units_per_em, density);

	// Advance the cursor by the correct amount
	x_c += glyph_dsc->adv_w * (int)scale / current_font->units_per_em;

	// printf("push_char(%8x) %8x %8x\n", dc, data_start, data_end);
}

static void set_x_cursor(int x_a, char *c, unsigned n, unsigned scale, unsigned align)
{
	int w_str = get_str_width(c, n, scale);
	if (align == A_RIGHT)
		x_c = x_a - w_str;
	else if (align == A_CENTER)
		x_c = x_a - w_str / 2;
	else
		x_c = x_a;
}

void push_str(int x_a, int y_a, char *c, unsigned n, unsigned align, unsigned scale, unsigned density)
{
	y_c = y_a;
	set_x_cursor(x_a, c, n, scale, align);

	while (*c && n > 0) {
        unsigned dc = utf8_dec(*c++);
		n--;
        if (dc == 0)
            continue;

		if (dc == '\n') {
			y_c -= scale;
			set_x_cursor(x_a, c, n, scale, align);
			continue;
		}

		push_char(dc, scale, density);
	}
	utf8_dec('\0');  // reset internal state
}
