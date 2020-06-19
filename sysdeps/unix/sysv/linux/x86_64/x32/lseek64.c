/* Linux lseek implementation, 64 bits off_t.  x86_64/x32 version.
   Copyright (C) 2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <unistd.h>
#include <sysdep.h>
#include <shlib-compat.h>

static inline off64_t
__lseek64_x32 (long int name, int fd, off64_t offset, int whence)
{
  unsigned long long int resultvar;
  register __syscall_arg_t a1 asm ("rdi") = fd;
  register __syscall_arg_t a2 asm ("rsi") = offset;
  register __syscall_arg_t a3 asm ("rdx") = whence;
  asm volatile ("syscall\n\t"
		: "=a" (resultvar)
		: "0" (name),  "r" (a1), "r" (a2), "r" (a3)
		: "memory", "cc", "r11", "cx");
  return resultvar < 0 ? __syscall_ret (resultvar) : resultvar;
}

#undef inline_syscall
#define inline_syscall(name, a1, a2, a3) __lseek64_x32 (name, a1, a2, a3)

/* Disable the llseek compat symbol.  */
#undef SHLIB_COMPAT
#define SHLIB_COMPAT(a, b, c) 0
#include <sysdeps/unix/sysv/linux/lseek64.c>
