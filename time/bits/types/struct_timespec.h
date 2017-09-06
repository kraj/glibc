#ifndef __timespec_defined
#define __timespec_defined 1

#include <bits/types.h>
#include <endian.h>

/* Use the original definition for 64-bit arches
   or when 64-bit-time by default has *not* been requested */ 
#if __WORDSIZE > 32 || ! defined(__USE_TIME_BITS64)
/* POSIX.1b structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  */
struct timespec
{
  __time_t tv_sec;		/* Seconds.  */
  __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
};
#else
/* Use the 64-bit-time timespec by default */
#define timespec __timespec64
# endif

/* 64-bit time version. To keep tings Posix-ish, we keep the nanoseconds
   field a signed long, but since Linux has a 64-bit signed int, we pad it
   with a 32-bit int, which should always be 0.
   Note that the public type has an anonymous bitfield as padding, so that
   it cannot be written into (or read from). */ 
#if BYTE_ORDER == BIG_ENDIAN
struct __timespec64
{
  __time64_t tv_sec;		/* Seconds */
  int tv_pad: 32;		/* Padding named for checking/setting */
  __syscall_slong_t tv_nsec;	/* Nanoseconds */
};
#else
struct __timespec64
{
  __time64_t tv_sec;		/* Seconds */
  __syscall_slong_t tv_nsec;	/* Nanoseconds */
  int tv_pad: 32;		/* Padding named for checking/setting */
};

#endif

#endif
