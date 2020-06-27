/* Assembly macros for C-SKY.
   Copyright (C) 2018-2020 Free Software Foundation, Inc.
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

#ifndef _LINUX_CSKY_SYSDEP_H
#define _LINUX_CSKY_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/sysv/linux/generic/sysdep.h>
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/csky/sysdep.h>

#ifndef __ASSEMBLER__
# include <stdint.h>
# include <errno.h>

static inline long int
__internal_syscall0 (long int name)
{
  register int a1 asm ("a0");
  register int nr asm ("r7") = name;
  asm volatile ("trap 0\n\t"
		: "=r" (a1)
		: "r" (nr)
		: "memory");
  return a1;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  register int a1 asm ("a0") = arg1;
  register int nr asm ("r7") = name;
  asm volatile ("trap 0\n\t"
		: "=r" (a1)
		: "r" (nr), "r" (a1)
		: "memory");
  return a1;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  register int a1 asm ("a0") = arg1;
  register int a2 asm ("a1") = arg2;
  register int nr asm ("r7") = name;
  asm volatile ("trap 0\n\t"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2)
		: "memory");
  return a1;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register int a1 asm ("a0") = arg1;
  register int a2 asm ("a1") = arg2;
  register int a3 asm ("a2") = arg3;
  register int nr asm ("r7") = name;
  asm volatile ("trap 0\n\t"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3)
		: "memory");
  return a1;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  register int a1 asm ("a0") = arg1;
  register int a2 asm ("a1") = arg2;
  register int a3 asm ("a2") = arg3;
  register int a4 asm ("a3") = arg4;
  register int nr asm ("r7") = name;
  asm volatile ("trap 0\n\t"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3), "r" (a4)
		: "memory");
  return a1;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  register int a1 asm ("a0") = arg1;
  register int a2 asm ("a1") = arg2;
  register int a3 asm ("a2") = arg3;
  register int a4 asm ("a3") = arg4;
  register int a5 asm ("r4") = arg5;
  register int nr asm ("r7") = name;
  asm volatile ("trap 0\n\t"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3), "r" (a4), "r" (a5)
		: "memory");
  return a1;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  register int a1 asm ("a0") = arg1;
  register int a2 asm ("a1") = arg2;
  register int a3 asm ("a2") = arg3;
  register int a4 asm ("a3") = arg4;
  register int a5 asm ("r4") = arg5;
  register int a6 asm ("r5") = arg6;
  register int nr asm ("r7") = name;
  asm volatile ("trap 0\n\t"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3), "r" (a4), "r" (a5),
		  "r" (a6)
		: "memory");
  return a1;
}

#undef HAVE_INTERNAL_BRK_ADDR_SYMBOL
#define HAVE_INTERNAL_BRK_ADDR_SYMBOL 1

#endif /* __ASSEMBLER__ */

/* Pointer mangling support.  */
#if (IS_IN (rtld) \
     || (!defined SHARED && (IS_IN (libc) || IS_IN (libpthread))))
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(dst, src, guard)			\
	grs	t0, 1f;					\
1:							\
	lrw	guard, 1b@GOTPC;			\
	addu	t0, guard;				\
	lrw	guard, __pointer_chk_guard_local@GOT;	\
	ldr.w	guard, (t0, guard << 0);		\
	ldw	guard, (guard, 0);			\
	xor	dst, src, guard;
#  define PTR_DEMANGLE(dst, src, guard) PTR_MANGLE (dst, src, guard)
#  define PTR_MANGLE2(dst, src, guard) \
	xor	dst, src, guard
#  define PTR_DEMANGLE2(dst, src, guard) PTR_MANGLE2 (dst, src, guard)
# else
extern uintptr_t __pointer_chk_guard_local;
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ __pointer_chk_guard_local)
#  define PTR_DEMANGLE(var) PTR_MANGLE (var)
# endif
#else
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(dst, src, guard)		\
	grs	t0, 1f;				\
1:						\
	lrw	guard, 1b@GOTPC;		\
	addu	t0, guard;			\
	lrw	guard, __pointer_chk_guard@GOT;	\
	ldr.w	guard, (t0, guard << 0);	\
	ldw	guard, (guard, 0);		\
	xor	dst, src, guard;
#  define PTR_DEMANGLE(dst, src, guard) PTR_MANGLE (dst, src, guard)
#  define PTR_MANGLE2(dst, src, guard) \
	xor	dst, src, guard
#  define PTR_DEMANGLE2(dst, src, guard) PTR_MANGLE2 (dst, src, guard)
# else
extern uintptr_t __pointer_chk_guard;
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ __pointer_chk_guard)
#  define PTR_DEMANGLE(var) PTR_MANGLE (var)
# endif
#endif

#endif /* linux/csky/sysdep.h */
