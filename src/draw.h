#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>
#include <stdbool.h>

// The higher this, the slower we draw
#define DENSITY_MULTIPLIER 4

// Coordinate values are clipped tp +- C_MAX
#define C_MAX 0x800

#define F_GOTO 0
#define F_LINETO 1
#define F_QBEZ 2
#define F_ARC 4
#define F_END 0xF

#define F_X_SHORT 1
#define F_Y_SHORT 2
#define F_X_POS 4
#define F_Y_POS 8

// draw-list data format:
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

// need to be implemented by DAC code
extern unsigned n_samples;

// Run through a serialized draw-list. Used for font-glyphs too.
void draw_blob(const uint8_t *p, unsigned n_bytes, int x_c, int y_c, int scale_a, int scale_div, int density);

// Updates the cursor to (x_a, y_a). Returns true if coordinates were clipped
bool push_goto(int x_a, int y_a);

// Draws a line from the current cursor to (x_b, y_b). updates the cursor
// Returns true if a sample was clipped
bool push_line(int x_b, int y_b, unsigned density);

void push_q_bezier(int x1, int y1, int x2, int y2, int density);

// Draws an ellipse, centered at (x_a, y_a) with radi (r_x, r_y)
// where alpha_length is the arc_length (360 deg is MAX_ANGLE)
// and alph_start is the start angle
// Returns true if a sample was clipped
bool push_circle(
    int x_a,
    int y_a,
    unsigned r_x,
    unsigned r_y,
    unsigned alpha_start,  // 0
    unsigned alpha_length,  // MAX_ANGLE
    unsigned density
);

bool output_sample(int x, int y, bool beam_on, int focus);


#endif
