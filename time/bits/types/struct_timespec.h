#ifndef _BITS_TYPES_STRUCT_TIMESPEC_H
#define _BITS_TYPES_STRUCT_TIMESPEC_H 1

#include <bits/types.h>

/* POSIX.1b structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  */
struct timespec
{
  __time_t tv_sec;		/* Seconds.  */
  __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
};

#endif
