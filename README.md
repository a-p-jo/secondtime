# `secondtime` v2.3.1

The metric system and SI units are wonderfully simple and used everywhere
for the convenience they offer. Except to measure time, which is measured
in annoying units that have no intuitive conversions to one another.

But screw that. Why not just denote time in seconds and represent with
scientific notation? But other people continue to use and expect the
larger units!

`secondtime` deals with this for you.
It's a command line utility that converts seconds to time in whatever units you want and vice versa.

It's written in standard C99/C11 so it should be trivial to compile
on any system. Once compiled, run it without arguments to get the diagnostics:

```
$ secondtime
Error : Incorrect argument(s).
Usage : secondtime <time> [format]

Help  : This program lets you use seconds as your unit of time.
        It has two basic functions:
        (1) Convert seconds to time units.
        (2) Convert time units to seconds.

        (1) To convert seconds, use it like so:

            	secondtime <number of seconds>

            The number of seconds may be any +ve real number.
            You can also express it in exponent form.
            Example: Convert 1.5 x 10^6 seconds.

                     $ secondtime 1.5e6
                     2w 3d 8h 40m

            (2 weeks, 3 days, 8 hours 40 minutes)

        (1.1) To convert to specific unit(s), use it like so:

              	secondtime <number of seconds> [unit(s)]

              To specify unit(s), just specify their suffix:
              years -> y, months -> M, weeks -> w, days -> d
              hours -> h, minutes -> m, seconds -> s
              Example: Convert 1.5 x 10^6 seconds to weeks and days.

                       $ secondtime 1.5e6 wd
                       2w 3.36111d

              (2 weeks, 3.36111 days)

        (2) To convert time to seconds, use it like so:

            	secondtime <time units with suffix>

            Write it like variables and coefficients in algebra.
            Example: Convert 2 weeks, 3 days, 8 hours and 40 minutes.

                     $ secondtime 2w3d8h40m
                     1.5e+06s

            (1.5 x 10^6 seconds)
```

**NOTE:** By default, a year means a gregorian year (365.2425 days); a month means 1/12th of that year. You can configure the year type in `config.h`.
