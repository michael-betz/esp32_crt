#include "font_data.h"

const font_t* f_all[] = {
	&f_arc,
	&f_hershey_romans,
	&f_hershey_scripts,
	&f_ubuntu,
	&f_pacifico,
	&f_lobster,
	&f_royal_brand,
	&f_weather_icons
};

const unsigned N_FONTS = sizeof(f_all) / sizeof(f_all[0]);
