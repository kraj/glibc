#ifndef __itimerspec_defined
#define __itimerspec_defined 1

#include <bits/types.h>
#include <bits/types/struct_timespec.h>

/* POSIX.1b structure for timer start values and intervals.  */
struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };

/* 64-bit interval timer spec */
struct __itimerspec64
{
  struct __timespec64 it_interval;
  struct __timespec64 it_value;
};

#endif
