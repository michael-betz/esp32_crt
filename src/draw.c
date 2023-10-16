#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "i2s.h"
#include "draw.h"
#include "print.h"  // for printing fixed-point numbers

// current cursor position
static int16_t x_cur = 0;
static int16_t y_cur = 0;


// https://stackoverflow.com/questions/34187171/fast-integer-square-root-approximation
static unsigned usqrt4(unsigned val) {
    unsigned a, b;

    if (val < 2) return val; /* avoid div/0 */

    a = 1255;       /* starting point is relatively unimportant */

    b = val / a; a = (a+b) /2;
    b = val / a; a = (a+b) /2;
    b = val / a; a = (a+b) /2;
    b = val / a; a = (a+b) /2;

    return a;
}


static unsigned push_line(int16_t x, int16_t y, uint16_t bright)
{
	unsigned n_samples = 0;

	// local copies
	int x_ = x_cur;
	int y_ = y_cur;

	int distx = x - x_;
	int disty = y - y_;

	unsigned dist;
	if (bright == 0)
		dist = 0;  // jump there directly
	else if (distx == 0)
		dist = abs(disty);
	else if (disty == 0)
		dist = abs(distx);
	else
		dist = usqrt4(distx * distx + disty * disty);

	int n = (dist * bright) >> FP;
	n = (n + FP_ROUND) >> FP;  // discard fractional part
	if (n > 1) {
		int dx = (distx << 8) / n;
		int dy = (disty << 8) / n;

		for (unsigned i = 0; i < n; i++) {
			push_sample(
				x_ + ((i * dx) >> 8),
				y_ + ((i * dy) >> 8),
				0,
				0
			);
			n_samples++;
		}
	}

	x_cur = x;
	y_cur = y;
	return n_samples;
}

unsigned push_list(draw_list_t *p, unsigned n_items)
{
	unsigned n_samples = 0;
	while (n_items--) {
		n_samples += push_line(p->x, p->y, p->brightness);
		p++;
	}
	return n_samples;
}
