/* Copyright (C) 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <sysdep.h>
#define preadv64 __redirect_preadv64
#include <sys/uio.h>
#include <sysdep-cancel.h>

#undef LOAD_ARGS_4
#define LOAD_ARGS_4(a1, a2, a3, a4)			\
  long long int __arg4 = (long long int) (a4);		\
  LOAD_ARGS_3 (a1, a2, a3)
#undef LOAD_REGS_4
#define LOAD_REGS_4					\
  register long long int _a4 asm ("r10") = __arg4;	\
  LOAD_REGS_3

ssize_t
preadv (int fd, const struct iovec *vector, int count, off_t offset)
{
  ssize_t result;
  if (SINGLE_THREAD_P)
    result = INLINE_SYSCALL (preadv, 4, fd, vector, count, offset);
  else
    {
      int oldtype = LIBC_CANCEL_ASYNC ();
      result = INLINE_SYSCALL (preadv, 4, fd, vector, count, offset);
      LIBC_CANCEL_RESET (oldtype);
    }
  return result;
}
#undef preadv64
strong_alias (preadv, preadv64)
