#ifndef FONT_DRAW_H
#define FONT_DRAW_H

#define FONT_TYPE_LIN 0
#define FONT_TYPE_ARC 1

typedef struct {
    uint8_t font_type;
    const int8_t *glyphs;
    const uint16_t *inds;
} font_t;

// scale is font-size. 100 is for ants, 300 is readable, 1000 is pretty huge
void push_str(int x_a, int y_a, char *c, unsigned n, unsigned align, unsigned scale, unsigned density);

#endif
