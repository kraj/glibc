/* Copyright (C) 1992-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>, August 1995.
   ARM changes by Philip Blundell, <pjb27@cam.ac.uk>, May 1997.

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

#ifndef _LINUX_ARM_SYSDEP_H
#define _LINUX_ARM_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/unix/arm/sysdep.h>

#include <bits/hwcap.h>

/* The ARM EABI user interface passes the syscall number in r7, instead
   of in the swi.  This is more efficient, because the kernel does not need
   to fetch the swi from memory to find out the number; which can be painful
   with separate I-cache and D-cache.  Make sure to use 0 for the SWI
   argument; otherwise the (optional) compatibility code for APCS binaries
   may be invoked.

   Linux takes system call args in registers:
	arg 1		r0
	arg 2		r1
	arg 3		r2
	arg 4		r3
	arg 5		r4	(this is different from the APCS convention)
	arg 6		r5
	arg 7		r6

   The compiler is going to form a call by coming here with arguments:
	arg 1		r0
	arg 2		r1
	arg 3		r2
	arg 4		r3
	arg 5		[sp]
	arg 6		[sp+4]
	arg 7		[sp+8]

   We need to shuffle values between R4..R6 and the stack so that the
   caller's v1..v3 and stack frame are not corrupted, and the kernel
   sees the right arguments.

   We must save and restore r7 (call-saved) for the syscall number.
   We never make function calls from inside here (only potentially
   signal handlers), so we do not bother with doubleword alignment.

   Just like the APCS syscall convention, the EABI syscall convention uses
   r0 through r6 for up to seven syscall arguments.  None are ever passed to
   the kernel on the stack, although incoming arguments are on the stack for
   syscalls with five or more arguments.

   The assembler will convert the literal pool load to a move for most
   syscalls.  */

#ifndef __ASSEMBLER__

#define VDSO_NAME  "LINUX_2.6"
#define VDSO_HASH  61765110

/* List of system calls which are supported as vsyscalls.  */
#define HAVE_CLOCK_GETTIME_VSYSCALL	"__vdso_clock_gettime"
#define HAVE_CLOCK_GETTIME64_VSYSCALL	"__vdso_clock_gettime64"
#define HAVE_GETTIMEOFDAY_VSYSCALL	"__vdso_gettimeofday"

static inline long int
__internal_syscall0 (long int name)
{
#if defined(__thumb__)
  register int a1 asm ("a1");
  register int nr asm ("ip") = name;
  asm volatile ("bl __libc_do_syscall"
		: "=r" (a1)
		: "r" (nr)
		: "memory", "lr");
  return a1;
#else
  register int a1 asm ("r0");
  register int nr asm ("r7") = name;
  asm volatile ("swi 0x0"
		: "=r" (a1)
		: "r" (nr)
		: "memory");
  return a1;
#endif
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
#if defined(__thumb__)
  register int a1 asm ("a1") = arg1;
  register int nr asm ("ip") = name;
  asm volatile ("bl __libc_do_syscall"
		: "=r" (a1)
		: "r" (nr), "r" (a1)
		: "memory", "lr");
  return a1;
#else
  register int a1 asm ("r0") = arg1;
  register int nr asm ("r7") = name;
  asm volatile ("swi 0x0"
		: "=r" (a1)
		: "r" (nr), "r" (a1)
		: "memory");
  return a1;
#endif
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
#if defined(__thumb__)
  register int a1 asm ("a1") = arg1;
  register int a2 asm ("a2") = arg1;
  register int nr asm ("ip") = name;
  asm volatile ("bl __libc_do_syscall"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2)
		: "memory", "lr");
  return a1;
#else
  register int a1 asm ("r0") = arg1;
  register int a2 asm ("a2") = arg2;
  register int nr asm ("r7") = name;
  asm volatile ("swi 0x0"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2)
		: "memory");
  return a1;
#endif
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
#if defined(__thumb__)
  register int a1 asm ("a1") = arg1;
  register int a2 asm ("a2") = arg1;
  register int a3 asm ("a3") = arg3;
  register int nr asm ("ip") = name;
  asm volatile ("bl __libc_do_syscall"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3)
		: "memory", "lr");
  return a1;
#else
  register int a1 asm ("r0") = arg1;
  register int a2 asm ("a2") = arg2;
  register int a3 asm ("a3") = arg3;
  register int nr asm ("r7") = name;
  asm volatile ("swi 0x0"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3)
		: "memory");
  return a1;
#endif
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
#if defined(__thumb__)
  register int a1 asm ("a1") = arg1;
  register int a2 asm ("a2") = arg1;
  register int a3 asm ("a3") = arg3;
  register int a4 asm ("a4") = arg4;
  register int nr asm ("ip") = name;
  asm volatile ("bl __libc_do_syscall"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3), "r" (a4)
		: "memory", "lr");
  return a1;
#else
  register int a1 asm ("r0") = arg1;
  register int a2 asm ("a2") = arg2;
  register int a3 asm ("a3") = arg3;
  register int a4 asm ("a4") = arg4;
  register int nr asm ("r7") = name;
  asm volatile ("swi 0x0"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3), "r" (a4)
		: "memory");
  return a1;
#endif
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
#if defined(__thumb__)
  register int a1 asm ("a1") = arg1;
  register int a2 asm ("a2") = arg1;
  register int a3 asm ("a3") = arg3;
  register int a4 asm ("a4") = arg4;
  register int a5 asm ("v1") = arg5;
  register int nr asm ("ip") = name;
  asm volatile ("bl __libc_do_syscall"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3), "r" (a4), "r" (a5)
		: "memory", "lr");
  return a1;
#else
  register int a1 asm ("r0") = arg1;
  register int a2 asm ("a2") = arg2;
  register int a3 asm ("a3") = arg3;
  register int a4 asm ("a4") = arg4;
  register int a5 asm ("v1") = arg5;
  register int nr asm ("r7") = name;
  asm volatile ("swi 0x0"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3), "r" (a4), "r" (a5)
		: "memory");
  return a1;
#endif
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
#if defined(__thumb__)
  register int a1 asm ("a1") = arg1;
  register int a2 asm ("a2") = arg1;
  register int a3 asm ("a3") = arg3;
  register int a4 asm ("a4") = arg4;
  register int a5 asm ("v1") = arg5;
  register int a6 asm ("v2") = arg6;
  register int nr asm ("ip") = name;
  asm volatile ("bl __libc_do_syscall"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3), "r" (a4), "r" (a5),
		  "r" (a6)
		: "memory", "lr");
  return a1;
#else
  register int a1 asm ("r0") = arg1;
  register int a2 asm ("a2") = arg2;
  register int a3 asm ("a3") = arg3;
  register int a4 asm ("a4") = arg4;
  register int a5 asm ("v1") = arg5;
  register int a6 asm ("v2") = arg6;
  register int nr asm ("r7") = name;
  asm volatile ("swi 0x0"
		: "=r" (a1)
		: "r" (nr), "r" (a1), "r" (a2), "r" (a3), "r" (a4), "r" (a5),
		  "r" (a6)
		: "memory");
  return a1;
#endif
}

#define SINGLE_THREAD_BY_GLOBAL	1

#endif	/* __ASSEMBLER__ */

#endif /* linux/arm/sysdep.h */
