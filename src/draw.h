#ifndef DRAW_H
#define DRAW_H

// number of bits in the fractional part of a fixed point number
#define FP 0
#define FP_ROUND 0  // (1 << (FP - 1))
#define FP_MAX 0x800

// Changes text alignment and anchor point
#define A_LEFT 0
#define A_CENTER 1
#define A_RIGHT 2

typedef struct {
    uint8_t type;
    uint8_t density;
    int16_t x;
    int16_t y;
} draw_list_t;

// need to be implemented by DAC code
extern unsigned n_samples;

// push all the samples of a draw-list to the DMA buffer once
// returns number of samples written
void push_list(draw_list_t *p, unsigned n_items);

// Updates the cursor to (x_a, y_a)
void push_goto(int x_a, int y_a);

// Draws a line from the current cursor to (x_b, y_b). updates the cursor
void push_line(int x_b, int y_b, unsigned density);

// Draws an ellipse, centered at (x_a, y_a) with radi (r_x, r_y)
// where alpha_length is the arc_length (360 deg is MAX_ANGLE)
// and alph_start is the start angle
void push_circle(
    int x_a,
    int y_a,
    unsigned r_x,
    unsigned r_y,
    unsigned alpha_start,  // 0
    unsigned alpha_length,  // MAX_ANGLE
    unsigned density
);

int push_char(int x_c, int y_c, char c, unsigned scale, unsigned density);

// scale is font-size. 100 is for ants, 300 is readable, 1000 is pretty huge
void push_str(int x_a, int y_a, char *c, unsigned align, unsigned scale, unsigned density);


#endif
