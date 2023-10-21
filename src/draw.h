#ifndef DRAW_H
#define DRAW_H

// number of bits in the fractional part of a fixed point number
#define FP 4
#define FP_ROUND (1 << (FP - 1))

typedef struct {
    uint8_t type;
    uint8_t density;
    int16_t x;
    int16_t y;
} draw_list_t;

// push all the samples of a draw-list to the DMA buffer once
// returns number of samples written
unsigned push_list(draw_list_t *p, unsigned n_items);

unsigned push_goto(int16_t x, int16_t y);

unsigned push_line(int16_t x, int16_t y, uint8_t density);

unsigned push_circle(
    uint16_t r_x,
    uint16_t r_y,
    uint16_t alpha_start,  // 0
    uint16_t alpha_length,  // MAX_ANGLE
    uint8_t density
);

unsigned push_char(char c, unsigned scale, unsigned density);

#endif
