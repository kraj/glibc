#ifndef _BITS_TYPES_STRUCT_TIMEVAL_H
#define _BITS_TYPES_STRUCT_TIMEVAL_H 1

#include <bits/types.h>

/* A time value that is accurate to the nearest
   microsecond but also has a range of years.  */
struct timeval
{
  __time_t tv_sec;		/* Seconds.  */
  __suseconds_t tv_usec;	/* Microseconds.  */
};
#endif
