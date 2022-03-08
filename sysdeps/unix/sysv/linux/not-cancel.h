/* Uncancelable versions of cancelable interfaces.  Linux/NPTL version.
   Copyright (C) 2003-2022 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#ifndef NOT_CANCEL_H
# define NOT_CANCEL_H

#include <fcntl.h>
#include <sysdep.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <time.h>

#if IS_IN (libc) || IS_IN (rtld)
/* Non cacellable openat syscall (LFS version).  */
hidden_proto2 (openat64, __openat64_nocancel)
/* Non cancellable read syscall.  */
hidden_proto2 (read, __read_nocancel)
/* Non cancellable pread syscall (LFS version).  */
hidden_proto2 (pread64, __pread64_nocancel)
/* Uncancelable write.  */
hidden_proto2 (write, __write_nocancel)
/* Uncancelable close.  */
hidden_proto2 (close, __close_nocancel)
/* Uncancelable fcntl.  */
hidden_proto2 (fcntl64, __fcntl64_nocancel)
#else
__typeof (openat64) __openat64_nocancel;
__typeof (read) __read_nocancel;
__typeof (pread64) __pread64_nocancel;
__typeof (write) __write_nocancel;
__typeof (close) __close_nocancel;
#endif

#define __open64_nocancel(args...) \
  __openat64_nocancel (AT_FDCWD, args)

/* Non cancellable close syscall that does not also set errno in case of
   failure.  */
static inline void
__close_nocancel_nostatus (int fd)
{
  __close_nocancel (fd);
}

/* Non cancellable writev syscall that does not also set errno in case of
   failure.  */
static inline void
__writev_nocancel_nostatus (int fd, const struct iovec *iov, int iovcnt)
{
  INTERNAL_SYSCALL_CALL (writev, fd, iov, iovcnt);
}

#endif /* NOT_CANCEL_H  */
