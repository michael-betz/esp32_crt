#ifndef FONT_DATA_H
#define FONT_DATA_H
#include <stdint.h>
#include <font_draw.h>

// gothgr  Gothic German
// gothgb  Gothic English
// gothit  Gothic Italian

// p       Plain (very small, no lower case)
// s       Simplex (plain, normal size, no serifs)
// d       Duplex (normal size, no serifs, doubled lines)
// c       Complex (normal size, serifs, doubled lines)
// t       Triplex (normal size, serifs, tripled lines)
// cs      Complex Small (Complex, smaller than normal size)

#define N_FONTS 1

extern const font_t f_arc;              // 0

extern const font_t* f_all[];

#endif
