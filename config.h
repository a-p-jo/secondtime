#ifndef SECONDTIME_CONFIG_H
#define SECONDTIME_CONFIG_H

/* Select a year type. Recompile to update. */
enum {
	/* The usual - 365, 366 days */
	NORMAL_YEAR, LEAP_YEAR,
	/* Calendar years */
	JULIAN_YEAR, GREGORIAN_YEAR,
	/* Astronomical years */
	TROPICAL_YEAR, SIDEREAL_YEAR,
	/* Select any of the above */
	SELECTED_YEAR = GREGORIAN_YEAR
};

#endif
