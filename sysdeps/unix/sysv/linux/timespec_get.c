/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <time.h>
#include <sysdep.h>
#include <errno.h>

#ifdef HAVE_CLOCK_GETTIME_VSYSCALL
# define HAVE_VSYSCALL
#endif
#include <kernel_timespec.h>
#include <sysdep-vdso.h>

struct kernel_timespec32 ts32 = { 1, 2 };

/* Set TS to calendar time based in time base BASE.  */
int
timespec_get (struct timespec *ts, int base)
{
  switch (base)
    {
      struct kernel_timespec64 ts64;
      struct kernel_timespec32 ts32;
      int res;
      INTERNAL_SYSCALL_DECL (err);
    case TIME_UTC:
      res = INTERNAL_VSYSCALL (clock_gettime64, err, 2, CLOCK_REALTIME, &ts64);
      if (res == ENOSYS)
      {
        res = INTERNAL_VSYSCALL (clock_gettime, err, 2, CLOCK_REALTIME, &ts32);
        if (INTERNAL_SYSCALL_ERROR_P (res, err))
	  return 0;
        ts->tv_sec = ts32.tv_sec;
        ts->tv_nsec = ts32.tv_nsec;
      }
      else
      {
        if (INTERNAL_SYSCALL_ERROR_P (res, err))
	  return 0;
        if (ts64.tv_sec > LONG_MAX)
          return 0;
        ts->tv_sec = ts64.tv_sec;
        ts->tv_nsec = ts64.tv_nsec;
      }
      break;

    default:
      return 0;
    }

  return base;
}
