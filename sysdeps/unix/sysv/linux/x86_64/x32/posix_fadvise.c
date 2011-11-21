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
#include <fcntl.h>
#include <sysdep.h>

#undef LOAD_ARGS_2
#define LOAD_ARGS_2(a1, a2)				\
  long long int __arg2 = (long long int) (a2);		\
  LOAD_ARGS_1 (a1)
#undef LOAD_REGS_2
#define LOAD_REGS_2					\
  register long long int _a2 asm ("rsi") = __arg2;	\
  LOAD_REGS_1

#undef LOAD_ARGS_3
#define LOAD_ARGS_3(a1, a2, a3)				\
  long long int __arg3 = (long long int) (a3);		\
  LOAD_ARGS_2 (a1, a2)
#undef LOAD_REGS_3
#define LOAD_REGS_3					\
  register long long int _a3 asm ("rdx") = __arg3;	\
  LOAD_REGS_2

/* Advice the system about the expected behaviour of the application with
   respect to the file associated with FD.  */

int
posix_fadvise (int fd, off_t offset, off_t len, int advise)
{
  INTERNAL_SYSCALL_DECL (err);
  int ret = INTERNAL_SYSCALL (fadvise64, err, 4, fd, offset, len, advise);
  if (INTERNAL_SYSCALL_ERROR_P (ret, err))
    return INTERNAL_SYSCALL_ERRNO (ret, err);
  return 0;
}
strong_alias (posix_fadvise, posix_fadvise64)
