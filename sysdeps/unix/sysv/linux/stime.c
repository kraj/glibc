/* Set the system clock on a Linux kernel 

   Copyright (C) 2017 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>		/* For NULL.  */
#include <sys/time.h>
#include <time.h>

/* Set the system clock to *WHEN.  */

int
stime (const time_t *when)
{
  struct timeval tv;

  if (when == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  tv.tv_sec = *when;
  tv.tv_usec = 0;
  return __settimeofday (&tv, (struct timezone *) 0);
}

/* 64-bit time version */

extern int __y2038_linux_support;

int
__stime_t64 (const __time64_t *when)
{
  struct timeval tv32;

  if (when == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (__y2038_linux_support)
  {
    /* TODO: implement 64-bit-time syscall case */
  }

  if (*when > INT_MAX)
    {
      __set_errno (EOVERFLOW);
      return -1;
    }

  tv32.tv_sec = *when;
  tv32.tv_usec = 0;
  return __settimeofday (&tv32, (struct timezone *) 0);
}
