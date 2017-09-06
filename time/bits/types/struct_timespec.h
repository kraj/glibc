#ifndef __timespec_defined
#define __timespec_defined 1

#include <bits/types.h>
#include <endian.h>

/* POSIX.1b structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  */
#if __WORDSIZE > 32 || ! defined(__USE_TIME_BITS64)
struct timespec
{
  __time_t tv_sec;		/* Seconds.  */
  __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
};
# elif BYTE_ORDER == BIG_ENDIAN
struct timespec
{
  __time64_t tv_sec;		/* Seconds.  */
  int: 32;			/* Hidden padding */
  __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
};
# else
struct timespec
{
  __time64_t tv_sec;		/* Seconds.  */
  __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
  int: 32;			/* Hidden padding */
};
# endif

#endif
