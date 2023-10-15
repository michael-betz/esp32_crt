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


static void push_line(int16_t x, int16_t y, uint16_t bright)
{
	// local copies
	int x_ = x_cur;
	int y_ = y_cur;

	printf("line from (%d, %d) to (%d, %d),", x_ >> FP, y_ >> FP, x >> FP, y >> FP);
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

	print_str(" dist: ");
	print_dec_fix(dist, FP, 2);

	int n = (dist * bright) >> FP;
	n = (n + FP_ROUND) >> FP;  // discard fractional part
	if (n > 1) {
		int dx = distx / n;
		int dy = disty / n;
		print_str(" dx: ");
		print_dec_fix(dx, FP, 2);
		print_str(" dy: ");
		print_dec_fix(dy, FP, 2);
		print_str("\n");

		for (unsigned i = 0; i < n - 1; i++) {
			x_ += dx;
			y_ += dy;
			push_sample(x_, y_, 0, 0);
		}
	} else {
		print_str("\n");
	}

	// don't accumulate rounding errors
	push_sample(x, y, 0, 0);
	x_cur = x;
	y_cur = y;
	return;
}


void push_list(draw_list_t *p, unsigned n_items)
{
	while (n_items--) {
		push_line(p->x, p->y, p->brightness);
		p++;
	}
}
