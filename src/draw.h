#ifndef DRAW_H
#define DRAW_H

// number of bits in the fractional part of a fixed point number
#define FP 4
#define FP_ROUND (1 << (FP - 1))

typedef struct {
    int16_t x;
    int16_t y;
    int16_t brightness;
} draw_list_t;

// push all the samples of a draw-list to the DMA buffer once
void push_list(draw_list_t *p, unsigned n_items);

#endif
