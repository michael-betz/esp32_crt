#ifndef DRAW_H
#define DRAW_H

// Coordinate values are clipped tp +- C_MAX
#define C_MAX 0x800

// Text alignment and horizontal anchor point
#define A_LEFT 0
#define A_CENTER 1
#define A_RIGHT 2

// Different blocks in the draw-list
#define T_NONE 0
#define T_LINE 1
#define T_POLY 2
#define T_CIRCLE 3
#define T_STRING 4
#define T_END 0xFE

typedef struct {
    uint8_t type;
    uint8_t density;
    int16_t x_b;
    int16_t y_b;
} line_t;  // 6 bytes

typedef struct {
    uint8_t type;
    uint8_t density;
    uint16_t len;  // Number of int16_t coordinate __pairs__ in pts
    int16_t pts[];
} poly_t;  // 4 bytes + 4 * len

typedef struct {
    uint8_t type;
    uint8_t density;
    int16_t x;
    int16_t y;
    int16_t r_x;
    int16_t r_y;
    uint8_t a_start;  // 255 == MAX_ANGLE
    uint8_t a_length;
} circle_t;  // 12 bytes

typedef struct {
    uint8_t type;
    uint8_t density;
    uint8_t align;
    uint16_t scale;
    int16_t x;
    int16_t y;
    uint16_t len;
    char c[];
} string_t;  // 12 bytes + len

// need to be implemented by DAC code
extern unsigned n_samples;

// push all the samples of a draw-list to the DMA buffer once
// returns number of samples written
void push_list(uint8_t *p, unsigned n_items);

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
