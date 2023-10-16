#ifndef FAST_SIN_H
#define FAST_SIN_H

#define MAX_ANGLE 4096

// initialize the angle look-up table
void init_lut(void);

// for angle alpha (360 deg = MAX_ANGLE) return its sine, scaled to INT_MAX
int get_sin(int alpha);

// return its cosine, scaled to INT_MAX
int get_cos(int alpha);

#endif
