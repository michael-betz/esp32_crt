#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <limits.h>
#include "fast_sin.h"

#define HALF_MAX_ANGLE (MAX_ANGLE / 2)
#define QUARTER_MAX_ANGLE (MAX_ANGLE / 4)

static int quarter_sin_lut[QUARTER_MAX_ANGLE];

void init_lut()
{
	for (unsigned i = 0; i < QUARTER_MAX_ANGLE; i++) {
		quarter_sin_lut[i] = sin(2.0 * M_PI * i / MAX_ANGLE) * INT_MAX;
	}
}

int get_sin(int alpha)
{
	alpha &= MAX_ANGLE - 1;
	int sign = alpha > HALF_MAX_ANGLE ? -1 : 1;
	alpha &= HALF_MAX_ANGLE - 1;
	if (alpha < QUARTER_MAX_ANGLE)
		return quarter_sin_lut[alpha] * sign;
	if (alpha == QUARTER_MAX_ANGLE)
		return INT_MAX * sign;
	if (alpha > QUARTER_MAX_ANGLE) {
		return quarter_sin_lut[2 * QUARTER_MAX_ANGLE - alpha] * sign;
	}
	return 0;
}

int get_cos(int alpha)
{
	return get_sin(alpha + QUARTER_MAX_ANGLE);
}
