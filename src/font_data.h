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

#define N_FONTS 15

extern font_t f_gothgbt;          // 0
extern font_t f_gothgrt;          // 1
extern font_t f_gothitt;          // 2
extern font_t f_italicc;          // 3
extern font_t f_italiccs;         // 4
extern font_t f_italict;          // 5
extern font_t f_romanc;           // 6
extern font_t f_romancs;          // 7
extern font_t f_romand;           // 8
extern font_t f_romanp;           // 9
extern font_t f_romans;           // 10
extern font_t f_romant;           // 11
extern font_t f_scriptc;          // 12
extern font_t f_scripts;          // 13
extern font_t f_arc;              // 14

extern font_t* f_all[];

#endif
