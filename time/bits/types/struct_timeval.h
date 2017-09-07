#ifndef __timeval_defined
#define __timeval_defined 1

#include <bits/types.h>

/* A time value that is accurate to the nearest
   microsecond but also has a range of years.  */
struct timeval
{
  __time_t tv_sec;		/* Seconds.  */
  __suseconds_t tv_usec;	/* Microseconds.  */
};

/* 64-bit time version. Here we can simply use 64-bit signed ints and
   still keep tings Posix-ish. */ 
struct __timeval64
{
  __time64_t tv_sec;		/* Seconds */
  __int64_t tv_usec;		/* Microseconds */
};
#endif
