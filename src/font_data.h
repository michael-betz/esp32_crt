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

extern font_t f_gothgbt;
extern font_t f_gothgrt;
extern font_t f_gothitt;
extern font_t f_italicc;
extern font_t f_italiccs;
extern font_t f_italict;
extern font_t f_romanc;
extern font_t f_romancs;
extern font_t f_romand;
extern font_t f_romanp;
extern font_t f_romans;
extern font_t f_romant;
extern font_t f_scriptc;
extern font_t f_scripts;
extern font_t f_arc;
#endif
