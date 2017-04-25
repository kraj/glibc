/* Change access and/or modification date of file.  Do not follow symbolic
   links.
   Copyright (C) 2007-2017 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sysdep.h>


int
lutimes (const char *file, const struct timeval tvp[2])
{
  /* The system call espects timespec, not timeval.  */
  struct timespec ts[2];
  if (tvp != NULL)
    {
      if (tvp[0].tv_usec < 0 || tvp[0].tv_usec >= 1000000
          || tvp[1].tv_usec < 0 || tvp[1].tv_usec >= 1000000)
	return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);

      TIMEVAL_TO_TIMESPEC (&tvp[0], &ts[0]);
      TIMEVAL_TO_TIMESPEC (&tvp[1], &ts[1]);
    }

  return INLINE_SYSCALL (utimensat, 4, AT_FDCWD, file, tvp ? ts : NULL,
			 AT_SYMLINK_NOFOLLOW);
}

/* 64-bit time version */

extern int __y2038_linux_support;

int
__lutimes64 (const char *file, const struct __timeval64 tvp[2])
{
  /* The system call expects timespec, not timeval.  */
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

      return INLINE_SYSCALL (utimensat64, 4, AT_FDCWD, file, &ts64,
			     AT_SYMLINK_NOFOLLOW);
    }

    return INLINE_SYSCALL (utimensat64, 4, AT_FDCWD, file, NULL,
			   AT_SYMLINK_NOFOLLOW);
  }

  if (tvp != NULL)
  {
    if (tvp[0].tv_usec < 0 || tvp[0].tv_usec >= 1000000
        || tvp[1].tv_usec < 0 || tvp[1].tv_usec >= 1000000)
      return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);

    ts32[0].tv_sec = tvp[0].tv_sec;
    ts32[0].tv_nsec = tvp[0].tv_usec * 1000;
    ts32[1].tv_sec = tvp[1].tv_sec;
    ts32[1].tv_nsec = tvp[1].tv_usec * 1000;

    return INLINE_SYSCALL (utimensat, 4, AT_FDCWD, file, &ts32,
                           AT_SYMLINK_NOFOLLOW);
  }

  return INLINE_SYSCALL (utimensat, 4, AT_FDCWD, file, NULL,
                         AT_SYMLINK_NOFOLLOW);
}
