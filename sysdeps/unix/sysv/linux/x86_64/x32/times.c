/* Linux times.  X32 version.
   Copyright (C) 2015-2020 Free Software Foundation, Inc.
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

#include <sys/times.h>
#include <sysdep.h>

/* Linux times system call returns 64-bit integer.  */
clock_t
__times (struct tms *buf)
{
  unsigned long long int resultvar;
  register __syscall_arg_t a1 asm ("rdi") = ARGIFY (buf);
  asm volatile ("syscall\n\t"
		: "=a" (resultvar)
		: "0" (__NR_times),  "r" (a1)
		: "memory", "cc", "r11", "cx");
  return __syscall_ret (resultvar);
}
weak_alias (__times, times)
