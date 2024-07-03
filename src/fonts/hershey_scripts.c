#include <stdint.h>
#include <stdio.h>
#include <font_draw.h>
// -----------------------------------
//  Hershey font, scripts
// -----------------------------------

static const uint8_t glyphs_scripts[5878] = {
 15,  8, 12, 19,  1,  1, 19,  2, 12, 15,  3, 12, 19,  3, 12, 15,  3, 13, 23,  1,  1, 19,  4, 12,
  3,  2,  6, 19,  1,  1, 23,  1,  1, 31,  1,  1, 27,  1,  1, 15,  7, 12, 19,  2,  7, 15,  3,  7,
 19,  3,  7, 15, 11,  7, 19,  2,  7, 15,  3,  7, 19,  3,  7, 15, 11, 16, 19,  7, 32, 15, 13, 32,
 19,  7, 32, 11,  6, 19, 21, 14,  3, 15,  6, 21, 14, 15, 12, 16, 19,  8, 29, 15, 13, 29, 19,  8,
 29, 15,  9, 21, 19,  1,  1, 23,  1,  1, 31,  1,  1, 26,  1, 27,  1,  2, 27,  1,  1, 27,  3,  1,
 17,  4, 19,  3,  1, 19,  2,  2, 18,  2, 23,  1,  2, 23,  1,  1, 23,  7,  4, 23,  2,  2, 11, 11,
  9, 23,  2,  2, 23,  7,  4, 23,  1,  1, 23,  1,  2, 18,  3, 19,  1,  2, 19,  1,  1, 19,  3,  1,
 17,  4, 27,  3,  1, 27,  1,  1, 27,  1,  2, 26,  1, 31,  1,  1, 23,  1,  1, 19,  1,  1, 15, 21,
 12, 19, 18, 21, 15,  5, 21, 23,  2,  2, 18,  2, 19,  1,  2, 19,  2,  1, 17,  2, 27,  2,  2, 26,
  2, 31,  1,  2, 31,  2,  1, 21,  2, 23,  2,  1, 23,  3,  1, 21,  3, 31,  3,  1, 31,  2,  1,  3,
  4, 14, 19,  2,  1, 19,  1,  2, 18,  2, 23,  2,  2, 21,  2, 31,  2,  1, 31,  1,  2, 26,  2, 27,
  2,  2, 17,  2, 15, 23,  4, 19,  1,  1, 23,  1,  1, 31,  1,  1, 26,  1, 27,  1,  1, 17,  1, 19,
  2,  1, 19,  2,  2, 19,  5,  8, 19,  2,  2, 19,  2,  1, 17,  3, 27,  3,  1, 27,  1,  2, 26,  2,
 31,  1,  2, 31,  1,  1, 31,  2,  1, 31,  5,  2, 31,  2,  1, 31,  2,  2, 31,  1,  2, 26,  2, 27,
  1,  2, 27,  2,  1, 19,  2,  1, 19,  1,  2, 18,  3, 23,  1,  6, 23,  1,  3, 23,  2,  3, 23,  2,
  2, 23,  2,  1, 21,  2, 31,  1,  2, 26,  1,  3, 15,  3, 27,  2,  1, 27,  1,  2, 26,  2, 31,  1,
  2, 31,  1,  1, 31,  6,  3, 11,  1,  4, 23,  1,  5, 23,  1,  3, 23,  2,  3, 23,  2,  2, 23,  2,
  1, 21,  2, 31,  1,  1, 15,  8, 10, 27,  1,  1, 31,  1,  1, 23,  1,  1, 18,  1, 19,  1,  2, 19,
  2,  2, 15, 15, 16, 19,  4,  3, 19,  3,  3, 19,  2,  3, 19,  2,  4, 19,  1,  5, 18,  4, 23,  1,
  5, 23,  1,  3, 23,  1,  2, 15,  5, 29, 19,  3,  4, 19,  2,  4, 19,  1,  3, 19,  1,  5, 18,  5,
 23,  1,  5, 23,  1,  3, 15,  9, 16, 23,  1,  2, 23,  1,  3, 23,  1,  5, 18,  4, 19,  1,  5, 19,
  2,  4, 19,  2,  3, 19,  3,  3, 19,  4,  3, 15,  9, 32, 23,  1,  3, 23,  1,  5, 18,  5, 19,  1,
  5, 19,  1,  3, 19,  2,  4, 19,  3,  4, 15, 10, 12, 18, 12, 11,  5,  9, 23, 10,  6, 10,  6, 19,
 10,  6, 15, 13,  9, 18, 18, 11,  9,  9, 21, 18,  7,  3,  9, 27,  1,  1, 31,  1,  1, 23,  1,  1,
 18,  1, 19,  1,  2, 19,  2,  2,  5,  4, 21, 18,  7,  5,  7, 19,  1,  1, 23,  1,  1, 31,  1,  1,
 27,  1,  1, 15, 24, 16, 19, 26, 32, 15, 12, 12, 19,  3,  1, 19,  2,  2, 19,  2,  3, 19,  1,  3,
 19,  1,  4, 18,  3, 23,  1,  3, 23,  1,  1, 23,  2,  1, 21,  2, 31,  3,  1, 31,  2,  2, 31,  2,
  3, 31,  1,  3, 31,  1,  4, 26,  3, 27,  1,  3, 27,  1,  1, 27,  2,  1, 17,  2,  0, 19,  2,  1,
 19,  2,  2, 19,  2,  3, 19,  1,  3, 19,  1,  4, 18,  3, 23,  1,  3, 23,  2,  2,  5,  2, 31,  2,
  1, 31,  2,  2, 31,  2,  3, 31,  1,  3, 31,  1,  4, 26,  3, 27,  1,  3, 27,  2,  2, 15, 12,  8,
 19,  5, 17, 15,  7, 21, 19,  6, 21, 15,  6, 21, 19,  3,  3, 19,  3,  2, 19,  2,  1, 15,  7,  3,
 19,  4,  2, 19,  3,  1, 15,  7,  8, 23,  1,  1, 19,  1,  1, 27,  1,  1, 26,  1, 31,  1,  2, 31,
  1,  1, 31,  3,  1, 21,  3, 23,  3,  1, 23,  1,  2, 18,  2, 19,  1,  2, 19,  2,  2, 19,  3,  2,
 19,  4,  2, 19,  3,  2, 19,  2,  2, 19,  2,  4, 15, 13, 21, 23,  2,  1, 23,  1,  2, 18,  2, 19,
  1,  2, 19,  2,  2, 19,  6,  4,  3,  6,  6, 31,  1,  1, 21,  2, 23,  5,  2, 21,  3, 31,  2,  1,
 31,  1,  2,  3, 11,  1, 23,  5,  3, 21,  3, 31,  2,  1, 31,  1,  3, 15,  7,  8, 23,  1,  1, 19,
  1,  1, 27,  1,  1, 26,  1, 31,  1,  2, 31,  1,  1, 31,  3,  1, 21,  3, 23,  3,  1, 23,  1,  2,
 18,  2, 19,  1,  2, 19,  3,  2, 19,  3,  1, 15,  3, 10, 23,  2,  1, 23,  1,  2, 18,  2, 19,  1,
  2, 19,  2,  2,  3,  5,  1, 21,  2, 23,  3,  1, 23,  1,  1, 23,  1,  2, 18,  3, 19,  1,  2, 19,
  1,  1, 19,  3,  1, 17,  4, 27,  3,  1, 27,  1,  1, 27,  1,  2, 26,  1, 31,  1,  1, 23,  1,  1,
 19,  1,  1, 15,  8,  7, 23,  2,  1, 23,  1,  1, 23,  1,  2, 18,  3, 19,  1,  2, 19,  1,  1, 19,
  2,  1, 15, 16, 11, 19,  6, 20, 15,  7, 21, 19,  6, 21, 15,  6, 21, 19, 15, 15, 21, 16, 15,  9,
 12, 19,  5, 10, 15,  5, 10, 21, 10,  3, 10,  1, 21,  5, 31,  5,  1,  3, 15, 10, 31,  1,  1, 31,
  3,  1, 21,  3, 23,  3,  1, 23,  1,  1, 23,  1,  2, 18,  3, 19,  1,  3, 19,  2,  2, 19,  3,  1,
 17,  3, 27,  3,  1, 27,  1,  1, 27,  1,  2, 26,  1, 31,  1,  1, 23,  1,  1, 19,  1,  1, 15,  8,
  9, 23,  2,  1, 23,  1,  1, 23,  1,  2, 18,  3, 19,  1,  3, 19,  2,  2, 19,  2,  1, 15, 17,  9,
 19,  1,  1, 23,  1,  1, 31,  1,  1, 26,  1, 27,  1,  2, 27,  2,  1, 17,  3, 19,  3,  1, 19,  2,
  2, 19,  2,  3, 19,  1,  3, 19,  1,  4, 18,  4, 23,  1,  2, 23,  1,  1, 23,  2,  1, 21,  3, 31,
  3,  1, 31,  2,  2, 31,  1,  2, 26,  3, 27,  1,  2, 27,  1,  1, 27,  2,  1, 17,  3, 19,  2,  1,
 19,  2,  2, 19,  1,  2, 15,  8, 14, 19,  2,  1, 19,  2,  2, 19,  2,  3, 19,  1,  3, 19,  1,  4,
 18,  5, 23,  1,  2,  7,  5,  1, 31,  2,  1, 31,  2,  2, 31,  1,  2, 26,  4, 27,  1,  2, 15,  6,
 12, 19,  2,  6, 15, 15,  6, 19,  1,  3, 19,  2,  3, 19,  5,  6, 19,  2,  3, 19,  1,  2, 19,  1,
  4, 15,  9, 15, 19,  6,  6, 19,  2,  3, 19,  1,  2, 19,  1,  4, 11,  1, 18, 31,  3,  3, 21,  2,
 23,  5,  3, 11,  9,  1, 31,  2,  1, 21,  2, 23,  5,  2, 21,  2, 31,  1,  1, 31,  1,  2, 15, 11,
 12, 19,  3,  1, 19,  1,  1, 19,  1,  2, 18,  3, 23,  1,  2, 23,  2,  1, 21,  3, 31,  4,  1, 31,
  1,  1, 31,  1,  2, 26,  3, 27,  1,  2, 27,  3,  1, 17,  3,  0, 19,  2,  1, 19,  1,  1, 19,  1,
  2, 18,  3, 23,  1,  2, 23,  1,  1,  5,  3, 31,  3,  1, 31,  1,  1, 31,  1,  2, 26,  3, 27,  1,
  2, 27,  2,  1,  3,  5, 10, 19,  4,  1, 19,  2,  2, 19,  1,  2, 18,  3, 23,  1,  2, 23,  3,  1,
 21,  4, 31,  4,  1, 31,  1,  1, 31,  1,  2, 26,  3, 27,  1,  2, 27,  1,  1, 27,  2,  1,  1,  3,
 19,  3,  1, 19,  2,  2, 19,  1,  2, 18,  3, 23,  1,  2, 23,  2,  1,  5,  4, 31,  3,  1, 31,  1,
  1, 31,  1,  2, 26,  4, 27,  1,  2, 15, 17,  5, 19,  1,  2, 19,  2,  2, 19,  2,  1, 17,  3, 27,
  2,  1, 27,  1,  1, 27,  1,  2, 26,  3, 31,  1,  2, 31,  2,  2, 31,  3,  1, 21,  3, 23,  2,  1,
 23,  1,  1, 23,  1,  2, 18,  4, 19,  1,  4, 19,  1,  3, 19,  2,  3, 19,  2,  2, 19,  3,  1, 17,
  3, 27,  2,  1, 27,  1,  2, 26,  1, 31,  1,  1, 23,  1,  1, 19,  1,  1, 15,  3,  7, 27,  1,  2,
 26,  4, 31,  1,  2, 31,  2,  2, 31,  2,  1,  7,  5,  1, 23,  1,  2, 18,  5, 19,  1,  4, 19,  1,
  3, 19,  2,  3, 19,  2,  2, 19,  2,  1, 15,  6,  5, 19,  1,  1, 23,  1,  1, 31,  1,  1, 27,  1,
  1,  3,  3, 12, 19,  1,  1, 23,  1,  1, 31,  1,  1, 15,  6,  5, 19,  1,  1, 23,  1,  1, 31,  1,
  1, 27,  1,  1,  3,  3, 14, 27,  1,  1, 31,  1,  1, 23,  1,  1, 18,  1, 19,  1,  2, 19,  2,  2,
 15, 20,  9, 19, 16,  9, 23, 16,  9, 15,  4,  3, 21, 18,  3, 18,  6, 21, 18, 15,  4,  9, 23, 16,
  9, 19, 16,  9, 15,  7,  8, 23,  1,  1, 19,  1,  1, 27,  1,  1, 26,  1, 31,  1,  2, 31,  1,  1,
 31,  3,  1, 21,  4, 23,  3,  1, 23,  1,  2, 18,  2, 19,  1,  2, 19,  1,  1, 19,  6,  2, 19,  2,
  1, 18,  2, 23,  1,  1, 21,  2, 15,  3, 14, 23,  2,  1, 23,  1,  2, 18,  2, 19,  1,  2, 19,  1,
  1, 19,  2,  1,  3,  6, 10, 19,  1,  1, 23,  1,  1, 31,  1,  1, 27,  1,  1, 15, 18,  4, 27,  1,
  2, 27,  2,  1, 17,  3, 19,  2,  1, 19,  1,  1, 19,  1,  3, 18,  3, 23,  1,  2, 23,  2,  1, 21,
  3, 31,  2,  1, 31,  1,  2, 11,  5,  8, 19,  2,  2, 19,  1,  3, 18,  3, 23,  1,  2, 23,  1,  1,
 15,  7, 11, 19,  1,  8, 18,  2, 23,  2,  1, 21,  2, 31,  2,  2, 31,  1,  3, 26,  2, 27,  1,  3,
 27,  1,  2, 27,  2,  2, 27,  2,  1, 27,  3,  1, 17,  3, 19,  3,  1, 19,  2,  1, 19,  2,  2, 19,
  1,  2, 19,  1,  3, 18,  3, 23,  1,  3, 23,  1,  2, 23,  2,  2, 23,  2,  1, 23,  3,  1, 21,  3,
 31,  3,  1, 31,  2,  1, 31,  1,  1, 11,  2, 13, 19,  1,  8, 18,  2, 23,  1,  1,  2,  9, 31,  2,
  1, 31,  3,  3, 31,  3,  4, 31,  4,  7, 31,  3,  6, 18, 21, 27,  1,  3, 27,  2,  3, 27,  2,  2,
 27,  3,  2, 17,  2, 19,  1,  1, 18,  2, 23,  1,  2, 23,  2,  2, 23,  3,  2, 23,  3,  1, 21,  5,
 15, 13, 10, 23,  1,  1, 18,  3, 19,  1,  4, 19,  1,  3, 19,  1,  2, 19,  2,  3, 19,  2,  2, 19,
  2,  1, 17,  1, 27,  1,  1, 26,  3, 31,  1,  5, 31,  1,  3, 31,  1,  2, 31,  2,  3, 31,  2,  2,
 31,  2,  1, 31,  3,  1, 21,  3, 23,  2,  1, 23,  1,  2, 18,  2, 19,  1,  2, 19,  1,  1, 19,  2,
  1, 19,  3,  1,  1,  1, 21,  1, 23,  3,  1, 23,  1,  1, 23,  1,  2, 18,  3, 19,  1,  2, 19,  1,
  1, 19,  2,  1, 17,  3, 27,  2,  1, 27,  1,  2, 15, 12,  6, 18,  1, 23,  1,  1, 21,  2, 31,  2,
  1, 31,  1,  2, 26,  2, 27,  1,  2, 27,  2,  1, 17,  3, 19,  3,  1, 19,  2,  2, 19,  2,  3, 19,
  1,  2, 19,  1,  4, 18,  4, 23,  1,  3, 23,  1,  1, 23,  2,  1, 21,  2, 31,  3,  1, 31,  2,  2,
 31,  1,  2, 15, 13, 12, 19,  2,  1, 19,  1,  2, 19,  1,  4, 19,  1,  6, 19,  1,  3, 19,  1,  2,
 19,  2,  2, 19,  2,  1, 17,  2, 27,  1,  1, 26,  2, 31,  1,  1, 21,  2, 23,  2,  1, 23,  2,  2,
 23,  3,  1, 21,  3, 31,  3,  1, 31,  2,  2, 31,  2,  4, 31,  1,  5, 26,  4, 27,  1,  3, 27,  1,
  1, 27,  2,  1, 17,  3, 19,  2,  2, 18,  2, 23,  1,  3, 23,  2,  3, 23,  2,  2, 23,  3,  2, 23,
  2,  1, 15, 14,  8, 18,  1, 23,  1,  1, 21,  2, 31,  1,  1, 26,  2, 27,  1,  2, 27,  3,  1, 17,
  4, 19,  3,  1, 19,  1,  2, 18,  3, 23,  1,  2, 23,  1,  1, 23,  3,  1, 17,  3, 19,  3,  1, 19,
  1,  1, 19,  1,  2, 18,  3, 23,  1,  2, 23,  1,  1, 23,  3,  1, 21,  3, 31,  3,  1, 31,  2,  2,
 31,  1,  2, 15, 10,  6, 17,  2, 27,  2,  1, 27,  1,  2, 31,  1,  2, 31,  3,  1, 21,  3, 23,  4,
  1, 21,  3, 31,  2,  1,  3,  5,  1, 19,  2,  7, 19,  2,  6, 19,  2,  4, 19,  2,  2, 19,  2,  1,
 17,  2, 27,  2,  1, 27,  1,  2, 26,  2, 31,  1,  1, 21,  2, 23,  2,  1, 15,  3,  6, 21,  9,  2,
  9, 31,  2,  1, 31,  4,  4, 31,  3,  5, 31,  1,  3, 31,  1,  4, 26,  3, 27,  1,  1, 17,  1, 19,
  1,  1, 19,  1,  2, 18,  3, 23,  1,  2, 23,  2,  1, 21,  4, 31,  3,  1, 31,  1,  1, 31,  1,  2,
 18,  6, 19,  1,  5, 19,  1,  2, 19,  2,  2, 19,  3,  1, 17,  4, 27,  3,  1, 27,  2,  2, 27,  1,
  2, 26,  2, 15,  7,  5, 27,  2,  1, 27,  1,  2, 26,  1, 31,  1,  2, 31,  2,  1, 21,  1, 23,  2,
  1, 23,  1,  2, 18,  2, 19,  1,  4, 19,  2,  6, 19,  2,  4, 19,  2,  2, 17,  2, 27,  1,  1, 26,
  2, 15,  6,  6, 31,  9,  3, 31,  2,  1, 31,  3,  2, 31,  2,  2, 31,  1,  2, 26,  1, 27,  1,  1,
 17,  1, 19,  2,  2, 19,  2,  4, 19,  2,  6, 19,  1,  5, 18,  3, 23,  1,  1, 21,  1, 31,  2,  1,
 31,  1,  1, 31,  2,  3,  7, 14,  4, 27,  2,  2, 27,  2,  3, 27,  1,  2, 27,  1,  3, 26,  3, 31,
  1,  2, 31,  1,  1, 21,  2, 23,  1,  1, 23,  1,  2, 18,  3, 19,  1,  5, 19,  2,  5, 19,  1,  2,
 19,  2,  2, 19,  2,  1, 17,  2, 27,  2,  1, 27,  1,  2, 26,  2, 31,  1,  1, 21,  2, 23,  2,  1,
  7, 10, 12, 27,  2,  3, 27,  2,  5, 27,  1,  6, 26,  6, 31,  1,  3, 31,  2,  1, 21,  2, 23,  1,
  1, 23,  1,  3, 18,  3, 19,  1,  5, 19,  3,  9, 19,  2,  6, 19,  1,  3, 19,  1,  2, 19,  2,  1,
 27,  1,  1, 26,  2, 31,  1,  3, 31,  2,  3, 31,  2,  2, 31,  3,  2, 31,  4,  2, 15,  7,  5, 27,
  2,  1, 27,  1,  2, 26,  1, 31,  1,  2, 31,  2,  1, 21,  1, 23,  2,  1, 23,  1,  2, 18,  2, 19,
  1,  4, 19,  2,  6, 19,  2,  4, 19,  2,  2, 17,  2, 27,  1,  1, 26,  2, 15, 23, 15, 26,  2, 27,
  1,  1, 17,  1, 19,  2,  1, 19,  2,  2, 19,  2,  3, 19,  2,  2, 19,  2,  1, 17,  2,  5,  2, 23,
  1,  2, 18,  7, 23,  1,  2, 23,  1,  1, 21,  1, 31,  2,  1, 31,  1,  1, 31,  2,  3,  5,  4, 21,
  2, 31,  4,  1, 31,  3,  2, 31,  2,  2, 31,  1,  2, 26,  3, 27,  1,  2, 17,  2, 19,  1,  1, 19,
  1,  2, 19,  1,  5, 19,  1,  5, 19,  1,  3, 19,  1,  2, 19,  2,  2, 19,  2,  1, 17,  2, 27,  1,
  1, 26,  2, 31,  1,  1, 21,  2, 23,  2,  1, 23,  3,  2, 23,  3,  1, 21,  2, 31,  3,  1, 31,  2,
  2, 15,  5,  5, 27,  2,  1, 27,  1,  2, 26,  1, 31,  1,  2, 31,  2,  1, 21,  1, 23,  2,  1, 23,
  1,  2, 18,  2, 19,  1,  5, 19,  1,  4, 19,  2,  7, 15,  2,  7, 31,  3,  8, 31,  2,  4, 31,  1,
  1, 31,  2,  1, 21,  1, 23,  2,  1, 23,  1,  2, 18,  2, 19,  1,  5, 19,  1,  4, 19,  2,  7, 15,
  2,  7, 31,  3,  8, 31,  2,  4, 31,  1,  1, 31,  2,  1, 21,  1, 23,  2,  1, 23,  1,  2, 18,  2,
 19,  1,  5, 19,  2,  7, 18,  3, 23,  1,  1, 21,  1, 31,  2,  1, 31,  1,  1, 31,  2,  3, 15,  5,
  5, 27,  2,  1, 27,  1,  2, 26,  1, 31,  1,  2, 31,  2,  1, 21,  1, 23,  2,  1, 23,  1,  2, 18,
  2, 19,  1,  5, 19,  1,  4, 19,  2,  7, 15,  2,  7, 31,  3,  8, 31,  2,  4, 31,  1,  1, 31,  2,
  1, 21,  2, 23,  2,  1, 23,  1,  2, 18,  2, 19,  1,  5, 19,  2,  7, 18,  3, 23,  1,  1, 21,  1,
 31,  2,  1, 31,  1,  1, 31,  2,  3, 15, 12, 12, 19,  3,  1, 19,  2,  2, 19,  2,  3, 19,  1,  2,
 19,  1,  4, 18,  4, 23,  1,  3, 23,  1,  1, 23,  2,  1, 21,  2, 31,  3,  1, 31,  2,  2, 31,  2,
  3, 31,  1,  2, 31,  1,  4, 26,  4, 27,  1,  3, 27,  1,  1, 27,  2,  1, 17,  2, 19,  2,  2, 18,
  3, 23,  1,  3, 23,  2,  3, 23,  2,  2, 23,  3,  2, 23,  2,  1, 15, 13, 10, 23,  1,  1, 18,  3,
 19,  1,  4, 19,  1,  3, 19,  1,  2, 19,  2,  3, 19,  2,  2, 19,  2,  1, 17,  1, 27,  1,  1, 26,
  3, 31,  1,  5, 31,  1,  3, 31,  1,  2, 31,  2,  3, 31,  2,  2, 31,  2,  1, 31,  3,  1, 21,  5,
 23,  2,  1, 23,  1,  1, 23,  1,  2, 18,  3, 19,  1,  2, 19,  1,  1, 19,  2,  1, 17,  3, 27,  2,
  1, 27,  1,  1, 15, 13,  6, 19,  1,  2, 19,  1,  1, 19,  2,  1, 17,  2, 27,  1,  2, 26,  2, 31,
  1,  3, 31,  2,  2, 31,  3,  1, 21,  3, 23,  2,  1, 23,  1,  2, 18,  4, 19,  1,  3, 19,  2,  3,
 19,  4,  4, 19,  3,  2, 19,  2,  1, 19,  3,  1, 17,  2, 27,  1,  1, 26,  2, 31,  1,  1, 21,  2,
 23,  2,  1, 23,  3,  2, 23,  3,  1, 21,  3, 31,  3,  1, 31,  2,  2, 15, 13, 10, 23,  1,  1, 18,
  3, 19,  1,  4, 19,  1,  3, 19,  1,  2, 19,  2,  3, 19,  2,  2, 19,  2,  1, 17,  1, 27,  1,  1,
 26,  3, 31,  1,  5, 31,  1,  3, 31,  1,  2, 31,  2,  3, 31,  2,  2, 31,  2,  1, 31,  3,  1, 21,
  4, 23,  2,  1, 23,  1,  1, 23,  1,  2, 18,  3, 19,  1,  2, 19,  1,  1, 19,  2,  1, 17,  3, 27,
  3,  1, 23,  1,  1, 23,  1,  2, 18,  5, 23,  1,  2, 23,  2,  1, 31,  2,  1, 31,  1,  1, 31,  2,
  3,  2,  9, 31,  2,  1, 31,  2,  2, 31,  3,  4, 31,  2,  3, 31,  2,  4, 31,  1,  3, 26,  3, 27,
  1,  1, 17,  1, 19,  1,  1, 19,  1,  2, 18,  2, 23,  1,  2, 23,  2,  2, 23,  3,  2, 23,  2,  2,
 23,  1,  2, 18,  2, 19,  1,  2, 19,  1,  1, 19,  3,  1, 17,  4, 27,  3,  1, 27,  2,  2, 27,  1,
  2, 26,  2, 15, 10,  6, 17,  2, 27,  2,  1, 27,  1,  2, 31,  1,  2, 31,  3,  1, 21,  3, 23,  4,
  1, 21,  3, 31,  2,  1,  3,  5,  1, 19,  2,  7, 19,  2,  6, 19,  2,  4, 19,  2,  2, 19,  2,  1,
 17,  2, 27,  2,  1, 27,  1,  2, 26,  2, 31,  1,  1, 21,  2, 23,  2,  1, 15,  5,  5, 27,  2,  1,
 27,  1,  2, 26,  1, 31,  1,  2, 31,  2,  1, 21,  1, 23,  2,  1, 23,  1,  2, 18,  2, 19,  1,  4,
 19,  1,  3, 19,  1,  4, 18,  2, 23,  1,  2, 23,  2,  1, 21,  2, 31,  2,  1, 31,  1,  1, 31,  2,
  4, 31,  3,  8, 31,  2,  7,  3,  2,  7, 19,  1,  4, 19,  1,  6, 18,  3, 23,  1,  1, 21,  1, 31,
  2,  1, 31,  1,  1, 31,  2,  3, 15,  5,  5, 27,  2,  1, 27,  1,  2, 26,  1, 31,  1,  2, 31,  2,
  1, 21,  1, 23,  2,  1, 23,  1,  2, 18,  2, 19,  1,  4, 19,  1,  3, 19,  1,  4, 18,  3, 23,  1,
  2, 21,  2, 31,  2,  1, 31,  3,  3, 31,  2,  3, 31,  2,  4, 31,  1,  3, 31,  1,  4, 26,  2, 27,
  1,  1, 17,  1, 19,  1,  1, 19,  1,  2, 18,  2, 23,  1,  3, 23,  2,  2, 23,  2,  1, 15,  5,  5,
 27,  2,  1, 27,  1,  2, 26,  1, 31,  1,  2, 31,  2,  1, 21,  1, 23,  2,  1, 23,  1,  2, 18,  3,
 19,  1, 15, 15, 10, 21, 19, 10, 21, 15, 10, 21, 19,  2, 21, 15, 14, 21, 19,  2,  1, 19,  3,  3,
 19,  3,  4, 19,  3,  6, 19,  3,  7, 15,  8,  6, 17,  2, 27,  1,  1, 26,  2, 31,  1,  2, 31,  2,
  1, 21,  2, 23,  2,  1, 23,  1,  2, 18,  3, 19,  2,  9, 18,  3, 23,  1,  2, 23,  2,  1, 21,  2,
 31,  2,  1, 31,  1,  2, 26,  2, 27,  1,  1, 17,  2, 15,  7, 12, 26,  2, 27,  1,  1, 17,  2, 19,
  2,  1, 19,  2,  2, 19,  2,  3, 19,  4,  9, 19,  2,  3, 19,  2,  2, 19,  2,  1, 17,  2, 27,  1,
  1, 26,  2, 15,  5,  5, 27,  2,  1, 27,  1,  2, 26,  1, 31,  1,  2, 31,  2,  1, 21,  1, 23,  2,
  1, 23,  1,  2, 18,  2, 19,  1,  4, 19,  1,  3, 19,  1,  4, 18,  2, 23,  1,  2, 23,  1,  1, 21,
  2, 31,  2,  1, 31,  2,  2, 31,  2,  3, 31,  1,  2, 31,  2,  6, 15,  2,  7, 19,  2,  7, 19,  3,
 10, 19,  2,  6, 19,  2,  5, 19,  2,  4, 19,  2,  1, 27,  1,  1, 26,  2, 31,  1,  3, 31,  2,  3,
 31,  3,  3, 31,  3,  2, 31,  5,  3, 15, 13,  6, 19,  1,  2, 19,  1,  1, 19,  2,  1, 17,  2, 27,
  1,  2, 26,  2, 31,  1,  3, 31,  2,  2, 31,  3,  1, 21,  3, 23,  2,  1, 23,  1,  2, 18,  4, 19,
  1,  3, 19,  2,  4, 19,  3,  3, 19,  4,  3, 19,  2,  1, 17,  3, 27,  1,  1, 26,  2, 31,  1,  1,
 21,  3, 23,  2,  1, 23,  1,  1, 23,  1,  2, 18,  3, 19,  1,  3, 19,  1,  2, 19,  2,  3, 19,  2,
  1, 27,  1,  1, 26,  2, 31,  1,  3, 31,  2,  3, 31,  3,  3, 31,  3,  2, 31,  6,  3, 15,  4, 16,
 18, 32, 15,  1, 32, 18, 32, 11,  1, 32, 21,  7,  3,  7, 32, 21,  7, 10, 12, 23, 14, 24, 15,  9,
 16, 18, 32, 15,  1, 32, 18, 32, 11,  7, 32, 21,  7,  3,  7, 32, 21,  7, 15,  6,  6, 31,  2,  3,
 23,  2,  3,  3,  7,  3, 31,  5,  5, 23,  5,  5, 11,  5,  5, 18, 17,  2, 11, 21, 16, 15,  9, 12,
 19,  2,  2, 19,  1,  2, 18,  1, 23,  1,  1, 31,  1,  1, 27,  1,  1,  7,  9,  3, 27,  1,  2, 27,
  2,  1, 17,  2, 19,  2,  1, 19,  1,  1, 19,  1,  2, 18,  2, 23,  1,  2, 23,  2,  1, 21,  2, 31,
  2,  1, 31,  1,  2, 31,  2,  6, 19,  1,  5, 18,  3, 23,  1,  1, 21,  1, 31,  2,  1, 31,  1,  1,
 31,  2,  3,  2,  4, 31,  2,  3, 31,  3,  5, 31,  1,  2, 31,  1,  3, 26,  2, 27,  1,  1, 19,  2,
  1, 19,  1,  2, 19,  1,  4, 19,  1,  7, 18,  6, 23,  1,  1, 21,  1, 31,  2,  1, 31,  2,  2, 31,
  1,  3, 26,  3, 23,  1,  4, 23,  1,  1, 21,  2, 31,  2,  1,  7,  7,  2, 26,  1, 27,  1,  1, 17,
  2, 19,  2,  1, 19,  1,  1, 19,  1,  2, 18,  2, 23,  1,  2, 23,  2,  1, 21,  3, 31,  3,  2, 31,
  2,  3,  7,  9,  3, 27,  1,  2, 27,  2,  1, 17,  2, 19,  2,  1, 19,  1,  1, 19,  1,  2, 18,  2,
 23,  1,  2, 23,  2,  1, 21,  2, 31,  2,  1, 31,  1,  2, 31,  6, 18,  3,  4, 12, 19,  1,  5, 18,
  3, 23,  1,  1, 21,  1, 31,  2,  1, 31,  1,  1, 31,  2,  3,  7,  1,  7, 31,  2,  1, 31,  1,  1,
 31,  1,  2, 26,  2, 27,  1,  1, 17,  1, 19,  2,  1, 19,  1,  2, 18,  3, 23,  1,  2, 23,  2,  1,
 21,  2, 31,  2,  1, 31,  1,  1, 31,  2,  3,  2,  4, 31,  4,  5, 31,  2,  3, 31,  1,  2, 31,  1,
  3, 26,  2, 27,  1,  1, 19,  2,  1, 19,  1,  2, 19,  2,  8, 19,  3,  9, 19,  3,  7, 19,  1,  3,
 18,  2, 23,  1,  1, 31,  2,  1, 31,  1,  3, 31,  1,  9, 23,  1,  1, 21,  2, 31,  2,  1, 31,  1,
  1, 31,  2,  3,  7,  9,  3, 27,  1,  2, 27,  2,  1, 17,  2, 19,  2,  1, 19,  1,  1, 19,  1,  2,
 18,  2, 23,  1,  2, 23,  2,  1, 21,  2, 31,  2,  1, 31,  1,  1, 15,  2,  7, 19,  2,  7, 19,  4,
 11, 19,  1,  2, 19,  2,  1, 27,  1,  1, 26,  2, 31,  1,  3, 31,  3,  3, 31,  3,  2, 31,  2,  1,
 31,  3,  2, 31,  3,  3,  2,  4, 31,  2,  3, 31,  3,  5, 31,  1,  2, 31,  1,  3, 26,  2, 27,  1,
  1, 19,  2,  1, 19,  1,  2, 19,  1,  4, 19,  1,  6, 19,  1,  8,  0, 31,  1,  3, 31,  1,  2, 31,
  2,  3, 31,  2,  1, 21,  2, 23,  1,  1, 18,  2, 19,  1,  3, 18,  2, 23,  1,  1, 21,  1, 31,  2,
  1, 31,  1,  1, 31,  2,  3, 15,  3,  5, 18,  1, 21,  1, 26,  1, 17,  1,  3,  3,  9, 31,  2,  4,
 19,  2,  6, 18,  2, 23,  1,  1, 21,  1, 31,  2,  1, 31,  1,  1, 31,  2,  3, 15,  3,  5, 18,  1,
 21,  1, 26,  1, 17,  1,  3,  3,  9, 31,  2,  4, 19,  6, 18, 19,  1,  2, 19,  2,  1, 27,  1,  1,
 26,  2, 31,  1,  3, 31,  3,  3, 31,  3,  2, 31,  2,  1, 31,  3,  2, 31,  3,  3,  2,  4, 31,  2,
  3, 31,  3,  5, 31,  1,  2, 31,  1,  3, 26,  2, 27,  1,  1, 19,  2,  1, 19,  1,  2, 19,  1,  4,
 19,  1,  6, 19,  1,  8,  0, 31,  1,  3, 31,  1,  2, 31,  2,  3, 31,  2,  1, 21,  2, 23,  1,  1,
 18,  2, 19,  2,  1, 17,  3,  0, 23,  2,  1, 23,  1,  3, 23,  1,  1, 21,  1, 31,  2,  1, 31,  1,
  1, 31,  2,  3,  2,  4, 31,  2,  3, 31,  3,  5, 31,  1,  2, 31,  1,  3, 26,  2, 27,  1,  1, 19,
  2,  1, 19,  1,  2, 19,  1,  4, 19,  1,  7, 18,  6, 23,  1,  1, 21,  1, 31,  2,  1, 31,  1,  1,
 31,  2,  3,  2,  4, 31,  2,  3, 31,  2,  1, 23,  1,  1, 18,  1, 19,  1,  4, 19,  1,  3, 15,  1,
  3, 31,  1,  2, 31,  2,  3, 31,  2,  1, 21,  2, 23,  1,  1, 18,  1, 19,  1,  4, 19,  1,  3, 15,
  1,  3, 31,  1,  2, 31,  2,  3, 31,  2,  1, 21,  2, 23,  1,  1, 18,  2, 19,  1,  3, 18,  2, 23,
  1,  1, 21,  1, 31,  2,  1, 31,  1,  1, 31,  2,  3,  2,  4, 31,  2,  3, 31,  2,  1, 23,  1,  1,
 18,  1, 19,  1,  4, 19,  1,  3, 15,  1,  3, 31,  1,  2, 31,  2,  3, 31,  2,  1, 21,  2, 23,  1,
  1, 18,  2, 19,  1,  3, 18,  2, 23,  1,  1, 21,  1, 31,  2,  1, 31,  1,  1, 31,  2,  3,  5,  6,
 17,  2, 19,  2,  1, 19,  1,  1, 19,  1,  2, 18,  2, 23,  1,  2, 23,  2,  1, 21,  2, 31,  2,  1,
 31,  1,  1, 31,  1,  2, 26,  2, 27,  1,  2, 27,  2,  1, 19,  1,  1, 18,  2, 23,  1,  2, 23,  2,
  1, 21,  3, 31,  2,  1, 31,  1,  1,  2,  4, 31,  2,  3, 31,  1,  2, 19,  1,  4, 19,  6, 18, 15,
  6, 18, 31,  1,  2, 31,  2,  1, 21,  2, 23,  2,  1, 23,  1,  2, 18,  2, 19,  1,  2, 19,  1,  1,
 19,  2,  1, 11,  4,  1, 23,  2,  1, 21,  3, 31,  3,  1, 31,  2,  1, 31,  3,  3,  7,  9,  3, 27,
  1,  2, 27,  2,  1, 17,  2, 19,  2,  1, 19,  1,  1, 19,  1,  2, 18,  2, 23,  1,  2, 23,  2,  1,
 21,  2, 31,  2,  1, 15,  3,  8, 19,  1,  3, 19,  2,  5, 19,  3,  7, 19,  1,  3, 18,  2, 23,  1,
  1, 31,  2,  1, 31,  1,  3, 26,  7, 31,  2,  1, 31,  3,  2, 31,  3,  3,  2,  4, 31,  2,  3, 31,
  1,  2, 18,  2, 21,  3, 23,  1,  1, 18,  2, 19,  1,  3, 18,  1, 23,  1,  1, 21,  1, 31,  2,  1,
 31,  1,  1, 31,  2,  3,  2,  4, 31,  2,  3, 31,  1,  2, 18,  2, 23,  2,  3, 23,  1,  2, 18,  2,
 19,  2,  1, 11,  4,  1, 23,  2,  1, 21,  4, 31,  2,  1, 31,  1,  1, 31,  2,  3,  2,  4, 31,  2,
  3, 31,  2,  4, 15,  3,  9, 19,  6, 18, 18,  2, 23,  1,  1, 21,  2, 31,  2,  1, 31,  1,  1, 31,
  2,  3, 11,  8,  8, 21,  7,  2,  4, 31,  2,  4, 19,  2,  6, 18,  2, 23,  1,  1, 21,  2, 31,  2,
  1, 31,  2,  2, 31,  2,  3, 15,  1,  3, 19,  2,  6, 18,  2, 23,  1,  1, 21,  1, 31,  2,  1, 31,
  1,  1, 31,  2,  3,  2,  4, 31,  2,  4, 19,  1,  5, 18,  3, 23,  1,  1, 21,  1, 31,  3,  1, 31,
  2,  2, 31,  1,  3, 26,  3,  0, 23,  1,  4, 23,  1,  1, 21,  2, 31,  2,  1,  5,  3, 19,  2,  2,
 19,  1,  3, 18,  2, 23,  1,  2, 21,  2, 31,  2,  1, 31,  2,  2, 15,  2,  6, 19,  2,  6, 18,  2,
 23,  1,  1, 21,  2, 31,  2,  1, 31,  2,  2, 31,  1,  3, 26,  3,  0, 23,  1,  4, 23,  1,  1, 21,
  2, 31,  2,  1,  2,  4, 31,  2,  3, 31,  2,  1, 21,  2, 23,  1,  1, 18,  7, 23,  1,  1, 21,  3,
 31,  3,  2, 31,  2,  3, 11,  3,  3, 27,  1,  1, 17,  2, 19,  1,  1, 19,  4,  7, 19,  1,  1, 17,
  2, 27,  1,  1,  2,  4, 31,  2,  4, 19,  2,  6, 18,  2, 23,  1,  1, 21,  2, 31,  2,  1, 31,  2,
  2, 31,  2,  3, 15,  1,  3, 19,  6, 18, 19,  1,  2, 19,  2,  1, 27,  1,  1, 26,  2, 31,  1,  3,
 31,  3,  3, 31,  3,  2, 31,  2,  1, 31,  3,  2, 31,  3,  3,  2,  4, 31,  2,  3, 31,  2,  1, 21,
  2, 23,  2,  2, 18,  2, 19,  1,  2, 19,  2,  2, 19,  3,  1, 23,  2,  1, 23,  1,  2, 18,  3, 19,
  1,  3, 19,  1,  2, 19,  2,  1, 27,  1,  1, 26,  2, 31,  1,  3, 31,  3,  3, 31,  3,  2, 31,  4,
  3, 31,  3,  3, 15,  9, 16, 19,  2,  1, 19,  1,  1, 19,  1,  2, 18,  2, 23,  1,  2, 23,  1,  1,
 23,  1,  2, 18,  2, 19,  2,  2, 15,  1, 14, 19,  1,  2, 18,  2, 23,  1,  2, 23,  1,  1, 23,  1,
  2, 18,  2, 19,  1,  2, 19,  4,  2, 23,  4,  2, 23,  1,  2, 18,  2, 19,  1,  2, 19,  1,  1, 19,
  1,  2, 18,  2, 23,  1,  2, 11,  1, 14, 23,  2,  2, 18,  2, 19,  1,  2, 19,  1,  1, 19,  1,  2,
 18,  2, 23,  1,  2, 23,  1,  1, 23,  2,  1, 15,  4, 16, 18, 32, 15,  5, 16, 23,  2,  1, 23,  1,
  1, 23,  1,  2, 18,  2, 19,  1,  2, 19,  1,  1, 19,  1,  2, 18,  2, 23,  2,  2, 11,  1, 14, 23,
  1,  2, 18,  2, 19,  1,  2, 19,  1,  1, 19,  1,  2, 18,  2, 23,  1,  2, 23,  4,  2, 19,  4,  2,
 19,  1,  2, 18,  2, 23,  1,  2, 23,  1,  1, 23,  1,  2, 18,  2, 19,  1,  2, 15,  1, 14, 19,  2,
  2, 18,  2, 23,  1,  2, 23,  1,  1, 23,  1,  2, 18,  2, 19,  1,  2, 19,  1,  1, 19,  2,  1,  7,
  3,  3, 26,  2, 31,  1,  3, 31,  2,  1, 21,  2, 23,  2,  1, 23,  4,  3, 23,  2,  1, 21,  2, 31,
  2,  1, 31,  1,  2,  3, 18,  2, 31,  1,  2, 31,  2,  1, 21,  2, 23,  2,  1, 23,  4,  3, 23,  2,
  1, 21,  2, 31,  2,  1, 31,  1,  3, 26,  2, 15,  6, 12, 19,  2,  1, 19,  1,  2, 18,  2, 23,  1,
  2, 23,  2,  1, 21,  2, 31,  2,  1, 31,  1,  2, 26,  2, 27,  1,  2, 27,  2,  1, 17,  2
};

// GLYPH DESCRIPTION
static const glyph_dsc_t glyph_dsc_scripts[96] = {
    {.end_index =     0, .adv_w =    16},
    {.end_index =    39, .adv_w =    11},
    {.end_index =    63, .adv_w =    18},
    {.end_index =    85, .adv_w =    21},
    {.end_index =   190, .adv_w =    21},
    {.end_index =   268, .adv_w =    24},
    {.end_index =   414, .adv_w =    26},
    {.end_index =   434, .adv_w =    11},
    {.end_index =   486, .adv_w =    15},
    {.end_index =   538, .adv_w =    15},
    {.end_index =   554, .adv_w =    17},
    {.end_index =   564, .adv_w =    26},
    {.end_index =   584, .adv_w =    11},
    {.end_index =   588, .adv_w =    26},
    {.end_index =   603, .adv_w =    10},
    {.end_index =   609, .adv_w =    22},
    {.end_index =   717, .adv_w =    21},
    {.end_index =   750, .adv_w =    21},
    {.end_index =   857, .adv_w =    21},
    {.end_index =   986, .adv_w =    21},
    {.end_index =  1006, .adv_w =    21},
    {.end_index =  1101, .adv_w =    21},
    {.end_index =  1222, .adv_w =    21},
    {.end_index =  1294, .adv_w =    21},
    {.end_index =  1449, .adv_w =    21},
    {.end_index =  1570, .adv_w =    21},
    {.end_index =  1597, .adv_w =    11},
    {.end_index =  1632, .adv_w =    11},
    {.end_index =  1641, .adv_w =    24},
    {.end_index =  1651, .adv_w =    26},
    {.end_index =  1660, .adv_w =    24},
    {.end_index =  1747, .adv_w =    21},
    {.end_index =  1892, .adv_w =    27},
    {.end_index =  1944, .adv_w =    20},
    {.end_index =  2052, .adv_w =    23},
    {.end_index =  2115, .adv_w =    20},
    {.end_index =  2210, .adv_w =    23},
    {.end_index =  2283, .adv_w =    20},
    {.end_index =  2351, .adv_w =    20},
    {.end_index =  2427, .adv_w =    23},
    {.end_index =  2526, .adv_w =    24},
    {.end_index =  2592, .adv_w =    17},
    {.end_index =  2660, .adv_w =    15},
    {.end_index =  2757, .adv_w =    24},
    {.end_index =  2833, .adv_w =    19},
    {.end_index =  2950, .adv_w =    33},
    {.end_index =  3033, .adv_w =    24},
    {.end_index =  3112, .adv_w =    21},
    {.end_index =  3196, .adv_w =    25},
    {.end_index =  3281, .adv_w =    22},
    {.end_index =  3385, .adv_w =    25},
    {.end_index =  3459, .adv_w =    20},
    {.end_index =  3522, .adv_w =    19},
    {.end_index =  3608, .adv_w =    24},
    {.end_index =  3693, .adv_w =    23},
    {.end_index =  3753, .adv_w =    28},
    {.end_index =  3843, .adv_w =    24},
    {.end_index =  3945, .adv_w =    23},
    {.end_index =  4053, .adv_w =    21},
    {.end_index =  4073, .adv_w =    14},
    {.end_index =  4078, .adv_w =    14},
    {.end_index =  4098, .adv_w =    14},
    {.end_index =  4121, .adv_w =    16},
    {.end_index =  4125, .adv_w =    16},
    {.end_index =  4145, .adv_w =    11},
    {.end_index =  4203, .adv_w =    16},
    {.end_index =  4263, .adv_w =    14},
    {.end_index =  4298, .adv_w =    11},
    {.end_index =  4359, .adv_w =    16},
    {.end_index =  4403, .adv_w =    10},
    {.end_index =  4468, .adv_w =     8},
    {.end_index =  4542, .adv_w =    15},
    {.end_index =  4615, .adv_w =    15},
    {.end_index =  4651, .adv_w =     7},
    {.end_index =  4700, .adv_w =     7},
    {.end_index =  4780, .adv_w =    14},
    {.end_index =  4827, .adv_w =     8},
    {.end_index =  4909, .adv_w =    25},
    {.end_index =  4966, .adv_w =    18},
    {.end_index =  5025, .adv_w =    14},
    {.end_index =  5084, .adv_w =    15},
    {.end_index =  5154, .adv_w =    15},
    {.end_index =  5190, .adv_w =    13},
    {.end_index =  5228, .adv_w =    11},
    {.end_index =  5263, .adv_w =     9},
    {.end_index =  5309, .adv_w =    15},
    {.end_index =  5347, .adv_w =    15},
    {.end_index =  5404, .adv_w =    21},
    {.end_index =  5452, .adv_w =    16},
    {.end_index =  5511, .adv_w =    15},
    {.end_index =  5572, .adv_w =    14},
    {.end_index =  5675, .adv_w =    14},
    {.end_index =  5680, .adv_w =     8},
    {.end_index =  5783, .adv_w =    14},
    {.end_index =  5843, .adv_w =    24},
    {.end_index =  5878, .adv_w =    14},
};

const font_t f_hershey_scripts = {
    .units_per_em = 28,
    .n_glyphs = 96,
    .glyphs = glyphs_scripts,
    .glyph_dsc = glyph_dsc_scripts,
    .map_start = 32,
    .map_n = 96,
    .map_unicode_table = NULL
};
