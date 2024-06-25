#ifndef FONT_DRAW_H
#define FONT_DRAW_H

// Text alignment and horizontal anchor point
#define A_LEFT 0
#define A_CENTER 1
#define A_RIGHT 2

// scale is font-size. 100 is for ants, 300 is readable, 1000 is pretty huge
void push_str(int x_a, int y_a, char *c, unsigned n, unsigned align, unsigned scale, unsigned density);

void set_font(unsigned index);

/** This describes a glyph. */
typedef struct {
    // uint16_t start_index;           /**< Start index of the vector */
    unsigned end_index;           /**< Last valid index of the vector */
    int16_t adv_w;            /**< Draw the next glyph after this width. */
    // int16_t ofs_x;                   /**< x offset of the bounding box*/
    // int16_t ofs_y;                  /**< y offset of the bounding box. Measured from the top of the line*/
} glyph_dsc_t;

typedef struct {
    const uint16_t units_per_em;
    const uint16_t n_glyphs;
    const uint8_t *glyphs;
    const glyph_dsc_t *glyph_dsc;

    // Each glyph is associated with its unicode value in `map_unicode_table`.
    // Glyphs shall be sorted by their unicode values, such that the table can be searched faster.
    // There is a simplification if the first N glyphs are in ASCII-code order, starting with ascii code 0x20.
    // Then map_n_ascii is set to N and these glyphs will not need an entry in `map_unicode_table`
    const unsigned map_n_ascii;
    const unsigned *map_unicode_table;
} font_t;

#endif
