#ifndef __timespec_defined
#define __timespec_defined 1

#include <bits/types.h>

/* POSIX.1b structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  */
struct timespec32
{
  __time_t tv_sec;		/* Seconds.  */
  __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
};

/* This one is for holding a Y2038-safe time value.  */
struct timespec64
{
  __time64_t tv_sec;			/* Seconds.  */
  __syscall_squad_t tv_nsec;	/* Nanoseconds.  */
};

#ifdef __USE_TIME_BITS64
#define timespec timespec64
#else
#define timespec timespec32
#endif

#endif
