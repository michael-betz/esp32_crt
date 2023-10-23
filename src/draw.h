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
unsigned push_list(draw_list_t *p, unsigned n_items);

void push_goto(int x_a, int y_a);

void push_line(int x_b, int y_b, unsigned density);

void push_circle(
    unsigned r_x,
    unsigned r_y,
    unsigned alpha_start,  // 0
    unsigned alpha_length,  // MAX_ANGLE
    unsigned density
);

void push_char(char c, unsigned scale, unsigned density);
void push_str(char *c, unsigned align, unsigned scale, unsigned density);


#endif
