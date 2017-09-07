/* Get the SCHED_RR interval for the named process.
  
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
#include <sched.h>
#include <sys/types.h>

extern int __y2038_linux_support;

int
__sched_rr_get_interval_t64 (pid_t pid, struct __timespec64 *t)
{
  struct timespec ts32;
  int result;

  if (t == NULL)
    {
      __set_errno(EINVAL);
      return -1;
    }

  if (__y2038_linux_support)
    {
      /* TODO: use 64-bit syscall */
    }

  result = sched_rr_get_interval(pid, &ts32);
  if (result == 0)
    {
      t->tv_sec = ts32.tv_sec;
      t->tv_nsec = ts32.tv_nsec;
      t->tv_pad = 0;
    }
  return result;
}
