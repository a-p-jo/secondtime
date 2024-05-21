#include <math.h>   /* NAN, fmodl, isfinite, isnan */
#include <stdio.h>  /* stderr, snprintf, sprintf, fprintf, printf */
#include <errno.h>  /* errno, ERANGE */
#include <stdlib.h> /* exit, EXIT_FAILURE, EXIT_SUCCESS, realloc, free */
#include <stdint.h> /* uint_least32_t, uint_least8_t, uint_fast8_t,
                     * uintmax_t, int_fast8_t */
#include <ctype.h>  /* isspace */

#include "sbomga.h" /* github.com/a-p-jo/darc/blob/main/sbomga/sbomga.h */
SBOMGA_IMPL(str, realloc, free, 0, char) /* Dynamic SSO string */

#include "config.h" /* Exports SELECTED year type */

#define SECS_IN_YR (                                     \
	SELECTED_YEAR == NORMAL_YEAR   ? 31536000L      :\
	SELECTED_YEAR == LEAP_YEAR     ? 31622400L      :\
	SELECTED_YEAR == JULIAN_YEAR   ? 31557600L      :\
	SELECTED_YEAR == GREGORIAN_YEAR? 3155695L       :\
	SELECTED_YEAR == TROPICAL_YEAR ? 31556925.216L  :\
	SELECTED_YEAR == SIDEREAL_YEAR ? 31558149.7635L :\
	NAN                                              \
)

typedef struct tm_unit {
	long double secs;
	char sfx;
} tm_unit;

static const tm_unit tm_units[] = {
	{SECS_IN_YR   , 'y'},
	{SECS_IN_YR/12, 'M'},
	{604800       , 'w'},
	{86400        , 'd'},
	{3600         , 'h'},
	{60           , 'm'},
	{1            , 's'}
};

#define LEN(x) (sizeof(x)/sizeof(x[0]))

/* s2str() format flag bits: Bit i corresponds to tm_units[i]. */
typedef uint_least8_t fmtflags;
#define fmtflags_ALL0 (       (fmtflags) {0}        )
#define fmtflags_ALL1 ( (fmtflags) {~fmtflags_ALL0} )

/* X is a fmtflags rvalue. 
 * N is an integer rvalue and N < bits in fmtflags
 */
#define fmtflags_GET(X, N) ((bool) (            \
	(X) &  (1 << (N))                       \
))
#define fmtflags_SET(X, N) ((fmtflags) {        \
	(X) |  (1 << (N))                       \
})
#define fmtflags_ANYSETAFTER(X, N) ((bool) (    \
	/* EXPLANATION:
	 * Say the low 7 bits of x are 0010100.
	 * We want to check if any bits after the 3rd bit are set (n=2).
	 * (1 << 3) gives 0001000.
	 * (1 << 3)-1 gives 0000111.
	 * ~((1 << 3)-1) gives 1111000.
	 * when we bitwise AND this with x:
	 * All bits until (and including) n are set to 0 (1&0 = 0, 0&0 = 0).
	 * All bits thereafter are unchanged because (0&1 = 0, 1&1 = 1).
	 * The resulting value, if any bits were after n were set,
	 * will evaluate to true when converted to bool.
	 */                                     \
	(X) & ~((1 << ((N)+1)) - 1)             \
))

/* Convert s seconds to str in specified format.
 *
 * If bit i of format is set, the tm_units[i] unit is converted to,
 * else not. 
 * Conversion begins with the largest unit,
 * converting the seconds to the greatest natural number of units,
 * then doing the same with the remaining seconds for the next largest unit,
 * and for the last unit convert remaining seconds to fractional number.
 * 
 * dst is cleared and may be realloc'd. Caller to handle deallocation.
 *
 * Returns true on success or false for formatting or reallocation errors
 * (in which case dst is indeterminate).
 */
static bool s2str(long double s, fmtflags format, str *dst)
{
	dst->len = 0;
	bool atleastone = false;

	for (uint_fast8_t i = 0; i < LEN(tm_units); i++) {
		if (!fmtflags_GET(format, i))
			continue; /* Ignore this unit */

		if (fmtflags_ANYSETAFTER(format, i)) {
			uintmax_t x = s/tm_units[i].secs; /* truncates */
			s = fmodl(s, tm_units[i].secs);
			if (x) {
				atleastone = true;

				#define WRITE_UNIT(fmtstr) do {            \
				/* Find space needed to print to dst */    \
				int n = snprintf(                          \
						NULL, 0,                   \
						(fmtstr),                  \
						x, tm_units[i].sfx         \
				) + 1;                                     \
				if (n < 0 || !str_reserve(dst,dst->len+n)) \
					return false;                      \
				/* Write to dst and update it's length */  \
				dst->len += sprintf(                       \
					str_arr(dst)+dst->len, (fmtstr),   \
					x, tm_units[i].sfx                 \
				);                                         \
				} while (0)

				WRITE_UNIT("%ju%c ");
			}
		} else { /* This is the last unit, convert to fractional. */
			long double x = s/tm_units[i].secs;
			/* If x is 0, write iff no units previously written */
			if (fpclassify(x) == FP_ZERO && atleastone)
				break;
			else
			/* https://stackoverflow.com/a/54162153/13651625 */
				WRITE_UNIT("%Lg%c");
			#undef WRITE_UNIT	
		}
	}
	return true;
}

/* If c is a suffix in tm_units, return its index, else return -1 */
static inline int_fast8_t isfx(char c)
{
	for (uint_fast8_t i = 0; i < LEN(tm_units); i++)
		if (c == tm_units[i].sfx)
			return i;
	return -1;
}

/* Converts src upto len chars to seconds, returning NAN on error */
static long double str2s(const char *restrict src, size_t len)
{
	long double s = 0;
	/* As unit is designated by suffix, iterate backwards */
	while (len --> 0) {
		int_fast8_t ci = isfx(src[len]);
		if (ci < 0) /* Non-suffix is last char, input is invalid */
			return NAN;
		else {
			const char *restrict cp = src+len;
			/* Rewind to start of value */
			while (len && isfx(src[len-1]) < 0)
				len--;
			char *ep;
			int olderrno = errno; errno = 0;
			/* Convert units to seconds and inc s */
			s += strtold(src+len, &ep) * tm_units[ci].secs;
			int newerrno = errno; errno = olderrno;
			if (
				ep != cp || signbit(s) 
				|| !isfinite(s) || newerrno == ERANGE
			)
				return NAN;
		}
	}
	return s;
}

/* Converts cstring to fmtflags, ignoring whitespace
 * but disallowing other non-suffix chars.
 * If s is NULL, enable all.
 */
static bool str2fmtflags(fmtflags *dst, const char *restrict src)
{
	if (!src) {
		*dst = fmtflags_ALL1;
		return true;
	}
	
	*dst = fmtflags_ALL0;
	for (char c; (c = *src); src++) {
		if (isspace(c))
			continue;
		
		int_fast8_t i = isfx(c);
		if (i < 0)
			return false;
		else
			*dst = fmtflags_SET(*dst, i);
	}
	return true;
}

int main(int argc, char **argv)
{
	int ret = EXIT_FAILURE;

	if (argc >= 2) {
		char *e;
		/* Assume argument is numerical */
		long double s = strtold(argv[1], &e);
		if (*e) { /* Not numerical, may be in time units */ 
			if (argc > 2)
				goto badargs;

			s = str2s(argv[1], strlen(argv[1]));
			if (isnan(s))
				fprintf(stderr, "Error: Invalid argument.\n");
			else {
				printf("%Lgs\n", s);
				ret = EXIT_SUCCESS;
			}
		} else if (signbit(s) || !isfinite(s) || errno == ERANGE)
			fprintf(stderr, "Error: Argument out of range.\n");
		else { /* Is numerical, convert seconds to time units */
			fmtflags fmt;
			if (!str2fmtflags(&fmt, argv[2]))
				goto badargs;

			str buf = {0};
			if (!s2str(s, fmt, &buf))
				fprintf(stderr, "Error: Couldn't convert.\n");
			else {
				printf("%s\n", str_arr(&buf));
				ret = EXIT_SUCCESS;
			}
			str_destroy(&buf);
		}
	} else badargs:
		fprintf(stderr,
		"Error : Incorrect argument(s).\n"
		"Usage : %s <time> [format]\n"
		"\n"
		"Help  : This program lets you use seconds as your unit of time.\n"
		"        It has two basic functions:\n"
		"        (1) Convert seconds to time units.\n"
		"        (2) Convert time units to seconds.\n"
		"\n"
		"        (1) To convert seconds, use it like so:\n"
		"\n"
		"            \t%s <number of seconds>\n"
		"\n"
		"            The number of seconds may be any +ve real number.\n"
		"            You can also express it in exponent form.\n"
		"            Example: Convert 1.5 x 10^6 seconds.\n"
		"\n"
		"                     $ %s 1.5e6\n"
		"                     2w 3d 8h 40m\n"
		"\n"
		"            (2 weeks, 3 days, 8 hours 40 minutes)\n"
		"\n"
		"        (1.1) To convert to specific unit(s), use it like so:\n"
		"\n"
		"              \t%s <number of seconds> [unit(s)]\n"
		"\n"
		"              To specify unit(s), just specify their suffix:\n"
		"              years -> y, months -> M, weeks -> w, days -> d\n"
		"              hours -> h, minutes -> m, seconds -> s\n"
		"              Example: Convert 1.5 x 10^6 seconds to weeks and days.\n"
		"\n"
		"                       $ %s 1.5e6 wd\n"
		"                       2w 3.36111d\n"
		"\n"
		"              (2 weeks, 3.36111 days)\n"
		"\n"
		"        (2) To convert time to seconds, use it like so:\n"
		"\n"
		"            \t%s <time units with suffix>\n"
		"\n"
		"            Write it like variables and coefficients in algebra.\n"
		"            Example: Convert 2 weeks, 3 days, 8 hours and 40 minutes.\n"
		"\n"
		"                     $ %s 2w3d8h40m\n"
		"                     1.5e+06s\n"
		"\n"
		"            (1.5 x 10^6 seconds)\n"
		, argv[0], argv[0], argv[0], argv[0], argv[0], argv[0], argv[0]);

	return ret;
}
