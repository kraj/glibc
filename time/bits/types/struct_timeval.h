#ifndef __timeval_defined
#define __timeval_defined 1

#include <bits/types.h>

/* A time value that is accurate to the nearest
   microsecond but also has a range of years.  */
#ifdef __USE_TIME_BITS64
struct timeval
{
  __time64_t tv_sec;		/* Seconds.  */
  __uint64_t tv_usec;   	/* Microseconds.  */
};
#else
struct timeval
{
  __time_t tv_sec;		/* Seconds.  */
  __suseconds_t tv_usec;	/* Microseconds.  */
};
#endif

#endif
