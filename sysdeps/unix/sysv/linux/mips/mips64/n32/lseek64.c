/* Linux lseek implementation, 64 bits off_t.  MIPS64n32 version.
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

#include <sysdep.h>

static inline long long int
__internal_syscall3_64 (long int name, __syscall_arg_t arg1,
			__syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register __syscall_arg_t s0 asm ("$16") = name;
  register __syscall_arg_t v0 asm ("$2");
  register __syscall_arg_t a0 asm ("$4") = arg1;
  register __syscall_arg_t a1 asm ("$5") = arg2;
  register __syscall_arg_t a2 asm ("$6") = arg3;
  register __syscall_arg_t a3 asm ("$7");
  asm volatile (".set\tnoreorder\n\t"
		MOVE32 "\t%0, %2\n\t"
		"syscall\n\t"
		".set reorder"
		: "=r" (v0), "=r" (a3)
		: "r" (s0), "r" (a0), "r" (a1), "r" (a2)
		: __SYSCALL_CLOBBERS);
  return a3 != 0 ? -v0 : v0;
}

#undef __internal_syscall_3
#define __internal_syscall_3(name, a1, a2, a3)                          \
  __internal_syscall3_64 (name, ARGIFY (a1), ARGIFY (a2), ARGIFY (a3))
#include <sysdeps/unix/sysv/linux/lseek64.c>
