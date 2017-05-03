/* Copyright (C) 2003-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sysdep.h>
#include "kernel-posix-timers.h"

int
__timerfd_gettime64 (int fd, struct __itimerspec64 *value)
{

  /* Try the 64-bit variant directly with the argument */
  int res = INLINE_SYSCALL (timerfd_gettime64, 2, fd, value);

  if (res == -1 && errno == ENOSYS)
  {
    struct itimerspec value32;
    res = INLINE_SYSCALL (timerfd_gettime, 2, fd, &value32);
    if (res == 0)
    {
      value->it_value.tv_sec = value32.it_value.tv_sec;
      value->it_value.tv_nsec = value32.it_value.tv_nsec;
      value->it_interval.tv_sec = value32.it_interval.tv_sec;
      value->it_interval.tv_nsec = value32.it_interval.tv_nsec;
    }
  }

  return res;
}
