#ifndef _BITS_TYPES_STRUCT_ITIMERSPEC_H
#define _BITS_TYPES_STRUCT_ITIMERSPEC_H 1

#include <bits/types.h>
#include <bits/types/struct_timespec.h>

/* POSIX.1b structure for timer start values and intervals.  */
struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };

#endif
