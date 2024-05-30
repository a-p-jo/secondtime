#ifndef SECONDTIME_CONFIG_H
#define SECONDTIME_CONFIG_H


/* 1: Select a year type. */

/* The usual - 365, 366 days */
#define NORMAL_YEAR    0
#define LEAP_YEAR      1
/* Calendar years */
#define JULIAN_YEAR    2
#define GREGORIAN_YEAR 3
/* Astronomical years */
#define TROPICAL_YEAR  4
#define SIDEREAL_YEAR  5

/* Select any of the above */
#define SELECTED_YEAR GREGORIAN_YEAR


/* 2: Select a style of printing floats. */

/* Print all digits possible for maximum accuracy */
#define ACCURATELY_PRINT_FLOATS 0
/* Balance accuracy and readability 
 * Enough digits for reasonable accuracy
 * Not so many digits that output is cluttered and too long
 */
#define READABLY_PRINT_FLOATS  1

/* Select any of the above */
#define HOW_TO_PRINT_FLOATS READABLY_PRINT_FLOATS

#endif
