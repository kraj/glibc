/* Syscall definitions, Linux s390 version.
   Copyright (C) 2019-2020 Free Software Foundation, Inc.
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

#ifndef __ASSEMBLY__

#define SINGLE_THREAD_BY_GLOBAL		1

#define VDSO_NAME  "LINUX_2.6.29"
#define VDSO_HASH  123718585

/* List of system calls which are supported as vsyscalls.  */
#ifdef __s390x__
#define HAVE_CLOCK_GETRES64_VSYSCALL	"__kernel_clock_getres"
#define HAVE_CLOCK_GETTIME64_VSYSCALL	"__kernel_clock_gettime"
#else
#define HAVE_CLOCK_GETRES_VSYSCALL	"__kernel_clock_getres"
#define HAVE_CLOCK_GETTIME_VSYSCALL	"__kernel_clock_gettime"
#endif
#define HAVE_GETTIMEOFDAY_VSYSCALL	"__kernel_gettimeofday"
#define HAVE_GETCPU_VSYSCALL		"__kernel_getcpu"

#ifndef __ASSEMBLER__
static inline long int
__internal_syscall0 (long int name)
{
  register long int r1 asm ("1") = name;
  register long int r2 asm ("2");
  asm volatile ("svc   0\n\t"
		: "=d" (r2)
		: "d" (r1)
		: "memory");
  return r2;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  register long int r1 asm ("1") = name;
  register long int r2 asm ("2") = arg1;
  asm volatile ("svc   0\n\t"
		: "+d" (r2)
		: "d" (r1)
		: "memory");
  return r2;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  register long int r1 asm ("1") = name;
  register long int r2 asm ("2") = arg1;
  register long int r3 asm ("3") = arg2;
  asm volatile ("svc   0\n\t"
		: "+d" (r2)
		: "d" (r1), "d" (r3)
		: "memory");
  return r2;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register long int r1 asm ("1") = name;
  register long int r2 asm ("2") = arg1;
  register long int r3 asm ("3") = arg2;
  register long int r4 asm ("4") = arg3;
  asm volatile ("svc   0\n\t"
		: "+d" (r2)
		: "d" (r1), "d" (r3), "d" (r4)
		: "memory");
  return r2;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  register long int r1 asm ("1") = name;
  register long int r2 asm ("2") = arg1;
  register long int r3 asm ("3") = arg2;
  register long int r4 asm ("4") = arg3;
  register long int r5 asm ("5") = arg4;
  asm volatile ("svc   0\n\t"
		: "+d" (r2)
		: "d" (r1), "d" (r3), "d" (r4), "d" (r5)
		: "memory");
  return r2;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  register long int r1 asm ("1") = name;
  register long int r2 asm ("2") = arg1;
  register long int r3 asm ("3") = arg2;
  register long int r4 asm ("4") = arg3;
  register long int r5 asm ("5") = arg4;
  register long int r6 asm ("6") = arg5;
  asm volatile ("svc   0\n\t"
		: "+d" (r2)
		: "d" (r1), "d" (r3), "d" (r4), "d" (r5), "d" (r6)
		: "memory");
  return r2;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  register long int r1 asm ("1") = name;
  register long int r2 asm ("2") = arg1;
  register long int r3 asm ("3") = arg2;
  register long int r4 asm ("4") = arg3;
  register long int r5 asm ("5") = arg4;
  register long int r6 asm ("6") = arg5;
  register long int r7 asm ("7") = arg6;
  asm volatile ("svc   0\n\t"
		: "+d" (r2)
		: "d" (r1), "d" (r3), "d" (r4), "d" (r5), "d" (r6), "d" (r7)
		: "memory");
  return r2;
}

#undef HAVE_INTERNAL_BRK_ADDR_SYMBOL
#define HAVE_INTERNAL_BRK_ADDR_SYMBOL 1

#endif /* __ASSEMBLER__  */

#endif
