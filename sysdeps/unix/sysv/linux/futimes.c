/* futimes -- change access and modification times of open file.  Linux version.
   Copyright (C) 2002-2017 Free Software Foundation, Inc.
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
#include <sysdep.h>
#include <string.h>
#include <time.h>
#include <utime.h>
#include <sys/time.h>
#include <_itoa.h>
#include <fcntl.h>


/* Change the access time of the file associated with FD to TVP[0] and
   the modification time of FILE to TVP[1].

   Starting with 2.6.22 the Linux kernel has the utimensat syscall which
   can be used to implement futimes.  */
int
__futimes (int fd, const struct timeval tvp[2])
{
  /* The utimensat system call expects timespec not timeval.  */
  struct timespec ts[2];
  if (tvp != NULL)
    {
      if (tvp[0].tv_usec < 0 || tvp[0].tv_usec >= 1000000
          || tvp[1].tv_usec < 0 || tvp[1].tv_usec >= 1000000)
	return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);

      TIMEVAL_TO_TIMESPEC (&tvp[0], &ts[0]);
      TIMEVAL_TO_TIMESPEC (&tvp[1], &ts[1]);
    }

  return INLINE_SYSCALL (utimensat, 4, fd, NULL, tvp ? &ts : NULL, 0);
}
weak_alias (__futimes, futimes)

/* 64-bit time version */

extern int __y2038_linux_support;

int
__futimes64 (int fd, const struct __timeval64 tvp[2])
{
  /* The utimensat system call expects timespec not timeval.  */
  struct __timespec64 ts64[2];
  struct timespec ts32[2];

  if (__y2038_linux_support)
  {
    if (tvp != NULL)
    {
      if (tvp[0].tv_usec < 0 || tvp[0].tv_usec >= 1000000
          || tvp[1].tv_usec < 0 || tvp[1].tv_usec >= 1000000)
	return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);

      ts64[0].tv_sec = tvp[0].tv_sec;
      ts64[0].tv_nsec = tvp[0].tv_usec * 1000;
      ts64[0].tv_pad = 0;
      ts64[1].tv_sec = tvp[1].tv_sec;
      ts64[1].tv_nsec = tvp[1].tv_usec * 1000;
      ts64[1].tv_pad = 0;

      return INLINE_SYSCALL (utimensat64, 4, fd, NULL, &ts64, 0);
    }

    return INLINE_SYSCALL (utimensat64, 4, fd, NULL, NULL, 0);
  }

  if (tvp != NULL)
  {
    if (tvp[0].tv_usec < 0 || tvp[0].tv_usec >= 1000000
        || tvp[1].tv_usec < 0 || tvp[1].tv_usec >= 1000000)
      return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);

    if (tvp[0].tv_sec > INT_MAX || tvp[1].tv_sec > INT_MAX)
      return INLINE_SYSCALL_ERROR_RETURN_VALUE (EOVERFLOW);

    ts32[0].tv_sec = tvp[0].tv_sec;
    ts32[0].tv_nsec = tvp[0].tv_usec * 1000;
    ts32[1].tv_sec = tvp[1].tv_sec;
    ts32[1].tv_nsec = tvp[1].tv_usec * 1000;

    return INLINE_SYSCALL (utimensat, 4, fd, NULL, &ts32, 0);
  }

  return INLINE_SYSCALL (utimensat, 4, fd, NULL, NULL, 0);
}
