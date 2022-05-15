#include <math.h>   /* NAN, fmodl(), isfinite(), isnan() */
#include <stdio.h>  /* stderr, snprintf(), fprintf(), printf() */
#include <errno.h>  /* errno, ERANGE */
#include <stdlib.h> /* exit(), EXIT_FAILURE, EXIT_SUCCESS, realloc(), free() */
#include <stdint.h> /* uint_least32_t, uint_fast8_t, uintmax_t, int_fast8_t */

#include "mga.h" /* github.com/a-p-jo/darc/blob/main/mga/mga.h */
MGA_IMPL(str, char) /* Dynamic string */

/* Number of seconds in a year, month, week, day, hour, minute and second */
static const uint_least32_t S_VALS[] = {31557600,2629800,604800,86400,3600,60,1};
static const char S_SFXS[] = {'y', 'M', 'w', 'd', 'h', 'm', 's'};

#define LEN(x) (sizeof(x)/sizeof(x[0]))

/* Convert s seconds to str in the form :
 * "<years>y <months>M <weeks>w <days>d <hours>h <mintues>m <seconds>s"
 * 
 * dst is cleared and may be realloc'd. Caller to handle deallocation.
 *
 * Returns true on success or false for formatting or reallocation errors
 * (in which case dst is indeterminate).
 */
static bool s_tostr(long double s, str *dst)
{
        dst->len = 0; 
        for (uint_fast8_t i = 0; i < LEN(S_VALS)-1; i++) {
                uintmax_t x = s/S_VALS[i]; s = fmodl(s, S_VALS[i]);
                if (x) {
                        /* Determine space needed to print to dst */
                        int n = snprintf(NULL, 0, "%ju%c ", x, S_SFXS[i]) + 1;
                        if (n < 0 || !str_reserve(dst, dst->len+n))
                                return false;
                        dst->len += snprintf(dst->arr+dst->len, n, "%ju%c ", x, S_SFXS[i]);
                }
        }
        if (s) {
                int n = snprintf(NULL, 0, "%Lgs", s) + 1;
                if (n < 0 || !str_reserve(dst, dst->len+n))
                        return false;
                dst->len += snprintf(dst->arr+dst->len, n, "%Lgs", s);
        }
        return true;
}

/* If c is in S_SFXS, return its index, else return -1 */
static inline int_fast8_t isfx(char c)
{
        for (uint_fast8_t i = 0; i < LEN(S_SFXS); i++)
                if (c == S_SFXS[i])
                        return i;
        return -1;
}

/* Converts src upto len chars to seconds, returning NAN on error. */
static long double s_frmstr(const char *restrict src, size_t len)
{
        long double s = 0;
        while (len-->0) { /* As unit is designated by suffix, iterate backwards */
                int_fast8_t ci = isfx(src[len]);
                if (ci < 0) /* Non-suffix char, input is invalid */
                        return NAN;
                else {
                        const char *restrict cp = src+len;
			for(; len && isfx(src[len-1]) < 0; len--) /* Rewind to start of value */
                                ;
                        char *ep;
			s += strtold(src+len, &ep) * S_VALS[ci];
                        if (ep != cp || !isfinite(s) || errno == ERANGE)
                                return NAN;
                }
        }
        return s;
}

int main(int argc, char **argv)
{
        int ret = EXIT_FAILURE;
        if (argc == 2) {
                char *e;
                long double s = strtold(argv[1], &e); /* Assume argument is numerical */
                if (*e) { /* Not numerical, may be in time units */ 
                        s = s_frmstr(argv[1], strlen(argv[1]));
                        if (isnan(s))
                                fprintf(stderr, "Error : Invalid argument.\n");
                        else
                                printf("%Lgs\n", s), ret = EXIT_SUCCESS;
                } else if (!isfinite(s) || errno == ERANGE)
                        fprintf(stderr, "Error : Argument out of valid range.\n");
                else { /* Is numerical, convert seconds to time units */
                        str buf = {.realloc = realloc, .free = free};
                        if (!s_tostr(s, &buf))
                                fprintf(stderr, "Error : Couldn't convert.\n");
                        else
                                printf("%s\n", buf.arr), ret = EXIT_SUCCESS;
                        str_destroy(&buf);
                }
        } else
                fprintf(stderr, "Error : Incorrect argument(s).\nUsage : %s <time>\n", argv[0]);
        return ret;
}

