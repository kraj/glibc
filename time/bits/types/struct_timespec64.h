#ifndef __timespec64_defined
#define __timespec64_defined 1

#include <bits/types.h>
#include <endian.h>

/* Y2036proof structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  To keep tings Posix-ish, we keep
   the nanoseconds field a signed long, but since Linux has a 64-bit signed int,
   we pad it with a 32-bit int, which should always be 0.
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
