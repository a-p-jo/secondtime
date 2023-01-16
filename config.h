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
