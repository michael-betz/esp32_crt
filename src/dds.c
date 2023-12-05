#include <limits.h>
#include "draw.h"
#include "fast_sin.h"
#include "dds.h"
#define N_DDS 4

static uint32_t phases[N_DDS] = {0, 0, 0, 0};
static uint32_t lut_type = 0x1002;
static uint32_t delta_fs[N_DDS] = {
	// Carrier
	0x1730080, 0x1730000,
	// Modulator
	0x0700000, 0x0700070,
};

void setup_dds(unsigned fcx, unsigned fcy, unsigned fmx, unsigned fmy, unsigned wfm)
{
	delta_fs[0] = fcx;
	delta_fs[1] = fcy;
	delta_fs[2] = fmx;
	delta_fs[3] = fmy;
	lut_type = wfm;
}

void draw_dds(unsigned n_samples)
{
	int32_t tmps[N_DDS], x_val = 0, y_val = 0;

	for (unsigned s_i = 0; s_i < n_samples; s_i++) {
		unsigned lut_t = lut_type;
		for (unsigned dds_i = 0; dds_i < N_DDS; dds_i++) {
			phases[dds_i] += delta_fs[dds_i];
			switch (lut_t & 0xF) {
			case 0:
				tmps[dds_i] = get_sin(phases[dds_i] >> 20);
				break;
			case 1:
				tmps[dds_i] = get_cos(phases[dds_i] >> 20);
				break;
			case 2:
				// note phases is UINT, INT_MAX refers to INT
				tmps[dds_i] = phases[dds_i] - INT_MAX;
				break;
			case 3:
				tmps[dds_i] = (phases[dds_i] > INT_MAX) ? INT_MAX : INT_MIN;
				break;
			case 4:
				tmps[dds_i] = INT_MAX;
				break;
			}
			lut_t >>= 4;
		}

		// // Amplitude modulation
		x_val = (tmps[0] >> 16) * (tmps[2] >> 16);
		y_val = (tmps[1] >> 16) * (tmps[3] >> 16);
		// x_val = ((int64_t)tmps[0] * (int64_t)tmps[2]) >> 32;
		// y_val = ((int64_t)tmps[1] * (int64_t)tmps[3]) >> 32;


		// normalize for full amplitude
		output_sample(x_val >> 20, y_val >> 20, 0x800, 0);
	}
	// printf("%08x %08x %08x %08x | %08x %08x\n", tmps[0], tmps[1], tmps[2], tmps[3], x_val, y_val);
}
