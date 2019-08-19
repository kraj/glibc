/* Compatibility definitions for `struct timeval' with 32-bit time_t.
   Copyright (C) 2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _TV32_COMPAT_H
#define _TV32_COMPAT_H 1

#include <features.h>

#include <bits/types.h>
#include <bits/types/time_t.h>
#include <bits/types/struct_timeval.h>
#include <bits/types/struct_timespec.h>
#include <bits/types/struct_rusage.h>

#include <stdint.h> // for INT32_MAX
#include <string.h> // for memset

#define TV_USEC_MAX 999999 // 10**6 - 1

/* A version of 'struct timeval' with 32-bit time_t.  */
struct timeval32
{
  int32_t tv_sec;
  int32_t tv_usec;
};

/* Structures containing 'struct timeval' with 32-bit time_t.  */
struct itimerval32
{
  struct timeval32 it_interval;
  struct timeval32 it_value;
};

struct rusage32
{
  struct timeval32 ru_utime;	/* user time used */
  struct timeval32 ru_stime;	/* system time used */
  long ru_maxrss;		/* maximum resident set size */
  long ru_ixrss;		/* integral shared memory size */
  long ru_idrss;		/* integral unshared data size */
  long ru_isrss;		/* integral unshared stack size */
  long ru_minflt;		/* page reclaims */
  long ru_majflt;		/* page faults */
  long ru_nswap;		/* swaps */
  long ru_inblock;		/* block input operations */
  long ru_oublock;		/* block output operations */
  long ru_msgsnd;		/* messages sent */
  long ru_msgrcv;		/* messages received */
  long ru_nsignals;		/* signals received */
  long ru_nvcsw;		/* voluntary context switches */
  long ru_nivcsw;		/* involuntary " */
};

/* Conversion functions.  If the seconds field of a timeval32 would
   overflow, they write { INT32_MAX, TV_USEC_MAX } to the output.  */

static inline void
TV32_TO_TV64 (struct timeval *restrict tv64,
              const struct timeval32 *restrict tv32)
{
  tv64->tv_sec = tv32->tv_sec;
  tv64->tv_usec = tv32->tv_usec;
}

static inline void
TV32_TO_TS64 (struct timespec *restrict ts64,
              const struct timeval32 *restrict tv32)
{
  ts64->tv_sec = tv32->tv_sec;
  ts64->tv_nsec = tv32->tv_usec * 1000;
}

static inline void
TV64_TO_TV32 (struct timeval32 *restrict tv32,
              const struct timeval *restrict tv64)
{
  if (__glibc_unlikely (tv64->tv_sec > (time_t) INT32_MAX))
    {
      tv32->tv_sec = INT32_MAX;
      tv32->tv_usec = TV_USEC_MAX;
    }
  else
    {
      tv32->tv_sec = tv64->tv_sec;
      tv32->tv_usec = tv64->tv_usec;
    }
}

static inline void
TS64_TO_TV32 (struct timeval32 *restrict tv32,
              const struct timespec *restrict ts64)
{
  if (__glibc_unlikely (ts64->tv_sec > (time_t) INT32_MAX))
    {
      tv32->tv_sec = INT32_MAX;
      tv32->tv_usec = TV_USEC_MAX;
    }
  else
    {
      tv32->tv_sec = ts64->tv_sec;
      tv32->tv_usec = ts64->tv_nsec / 1000;
    }
}

static inline void
RUSAGE64_TO_RUSAGE32 (struct rusage32 *restrict r32,
                      const struct rusage *restrict r64)
{
  /* Make sure the entire output structure is cleared, including
     padding and reserved fields.  */
  memset (r32, 0, sizeof *r32);

  TV64_TO_TV32 (&r32->ru_utime, &r64->ru_utime);
  TV64_TO_TV32 (&r32->ru_stime, &r64->ru_stime);
  r32->ru_maxrss   = r64->ru_maxrss;
  r32->ru_ixrss    = r64->ru_ixrss;
  r32->ru_idrss    = r64->ru_idrss;
  r32->ru_isrss    = r64->ru_isrss;
  r32->ru_minflt   = r64->ru_minflt;
  r32->ru_majflt   = r64->ru_majflt;
  r32->ru_nswap    = r64->ru_nswap;
  r32->ru_inblock  = r64->ru_inblock;
  r32->ru_oublock  = r64->ru_oublock;
  r32->ru_msgsnd   = r64->ru_msgsnd;
  r32->ru_msgrcv   = r64->ru_msgrcv;
  r32->ru_nsignals = r64->ru_nsignals;
  r32->ru_nvcsw    = r64->ru_nvcsw;
  r32->ru_nivcsw   = r64->ru_nivcsw;
}

#endif /* tv32-compat.h */
