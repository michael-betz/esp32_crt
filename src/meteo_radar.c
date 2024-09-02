#include <stdio.h>
#include "fonts/font_data.h"
#include "draw.h"

// Measured radar data and predictions are given in separate files ...
//
// The measured rain from radar data, use the following URL
// https://www.meteoswiss.admin.ch/product/output/radar/rzc/radar_rzc.20240902_0915.json
// The time is given as UTC time (-2h of local time in summer)
// time resolution is 5 minutes: 0915, 0920, 0925 are valid
// The data seems to be valid and available with about 10 - 15 min of delay
//
// Each forecast prediction gets its own `version` which looks like a timestamp. Predictions are updated
// every 5 minutes or so. The latest valid version can be get from:
// https://www.meteoswiss.admin.ch/product/output/versions.json
//
// The version for the rain prediction is this one:
// {"inca/precipitation/rate": "20240902_0949"}
//
// Then get the rain data for 13:45 with the following URL
// https://www.meteoswiss.admin.ch/product/output/inca/precipitation/rate/version__20240902_0949/rate_20240902_1345.json


void meteo_radar()
{
	draw_blob(
		svg_switzerland_simple_simple_svg,
		sizeof(svg_switzerland_simple_simple_svg),
		0, 0,
		1, 1,
		200
	);
}

