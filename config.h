#ifndef SECONDTIME_CONFIG_H
#define SECONDTIME_CONFIG_H

/* Select a year type. Recompile to update. */
enum {
	/* The usual - 365, 366 days */
	NORMAL, LEAP,
	/* Calendar years */
	JULIAN, GREGORIAN,
	/* Astronomical years */
	TROPICAL, SIDEREAL,
	/* Select any of the above */
	SELECTED = GREGORIAN
};

#endif
