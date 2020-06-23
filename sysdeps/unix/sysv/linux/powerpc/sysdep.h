/* Syscall definitions, Linux PowerPC generic version.
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

#ifndef _LINUX_POWERPC_SYSDEP_H
#define _LINUX_POWERPC_SYSDEP_H 1

#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/unix/powerpc/sysdep.h>
#include <errno.h>

/* For Linux we can use the system call table in the header file
       /usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)  __NR_##syscall_name

#ifndef __ASSEMBLER__
static inline long int
__internal_syscall0 (long int name)
{
  register long int r0  __asm__ ("r0") = name;
  register long int r3  __asm__ ("r3");
  asm volatile ("sc\n\t"
		"neg  9, %1\n\t"
		"isel %1, 9, %1, 3\n\t"
		: "+r" (r0), "=r" (r3)
		:
		: "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
		  "cr0", "memory");
  return r3;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  register long int r0  __asm__ ("r0") = name;
  register long int r3  __asm__ ("r3") = arg1;
  asm volatile ("sc\n\t"
		"neg  9, %1\n\t"
		"isel %1, 9, %1, 3\n\t"
		: "+r" (r0), "+r" (r3)
		:
		: "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
		  "cr0", "memory");
  return r3;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  register long int r0  __asm__ ("r0") = name;
  register long int r3  __asm__ ("r3") = arg1;
  register long int r4  __asm__ ("r4") = arg2;
  asm volatile ("sc\n\t"
		"neg  9, %1\n\t"
		"isel %1, 9, %1, 3\n\t"
		: "+r" (r0), "+r" (r3), "+r" (r4)
		:
		: "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
		  "cr0", "memory");
  return r3;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register long int r0  __asm__ ("r0") = name;
  register long int r3  __asm__ ("r3") = arg1;
  register long int r4  __asm__ ("r4") = arg2;
  register long int r5  __asm__ ("r5") = arg3;
  asm volatile ("sc\n\t"
		"neg  9, %1\n\t"
		"isel %1, 9, %1, 3\n\t"
		: "+r" (r0), "+r" (r3), "+r" (r4), "+r" (r5)
		:
		: "r6", "r7", "r8", "r9", "r10", "r11", "r12",
		  "cr0", "memory");
  return r3;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  register long int r0  __asm__ ("r0") = name;
  register long int r3  __asm__ ("r3") = arg1;
  register long int r4  __asm__ ("r4") = arg2;
  register long int r5  __asm__ ("r5") = arg3;
  register long int r6  __asm__ ("r6") = arg4;
  asm volatile ("sc\n\t"
		"neg  9, %1\n\t"
		"isel %1, 9, %1, 3\n\t"
		: "+r" (r0), "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6)
		:
		: "r7", "r8", "r9", "r10", "r11", "r12",
		  "cr0", "memory");
  return r3;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  register long int r0  __asm__ ("r0") = name;
  register long int r3  __asm__ ("r3") = arg1;
  register long int r4  __asm__ ("r4") = arg2;
  register long int r5  __asm__ ("r5") = arg3;
  register long int r6  __asm__ ("r6") = arg4;
  register long int r7  __asm__ ("r7") = arg5;
  asm volatile ("sc\n\t"
		"neg  9, %1\n\t"
		"isel %1, 9, %1, 3\n\t"
		: "+r" (r0), "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6),
		  "+r" (r7)
		:
		: "r8", "r9", "r10", "r11", "r12",
		  "cr0", "memory");
  return r3;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  register long int r0  __asm__ ("r0") = name;
  register long int r3  __asm__ ("r3") = arg1;
  register long int r4  __asm__ ("r4") = arg2;
  register long int r5  __asm__ ("r5") = arg3;
  register long int r6  __asm__ ("r6") = arg4;
  register long int r7  __asm__ ("r7") = arg5;
  register long int r8  __asm__ ("r8") = arg6;
  asm volatile ("sc\n\t"
		"neg  9, %1\n\t"
		"isel %1, 9, %1, 3\n\t"
		: "+r" (r0), "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6),
		  "+r" (r7), "+r" (r8)
		:
		: "r9", "r10", "r11", "r12",
		  "cr0", "memory");
  return r3;
}
#endif

/* Pointer mangling support.  */
#if IS_IN (rtld)
/* We cannot use the thread descriptor because in ld.so we use setjmp
   earlier than the descriptor is initialized.  */
#else
# include <tcb-offsets.h>
# ifdef __ASSEMBLER__
#  if defined(__PPC64__) || defined(__powerpc64__)
#   define LOAD  ld
#   define TPREG r13
#  else
#   define LOAD  lwz
#   define TPREG r2
#  endif
#  define PTR_MANGLE(reg, tmpreg) \
	LOAD	tmpreg,POINTER_GUARD(TPREG); \
	xor	reg,tmpreg,reg
#  define PTR_MANGLE2(reg, tmpreg) \
	xor	reg,tmpreg,reg
#  define PTR_MANGLE3(destreg, reg, tmpreg) \
	LOAD	tmpreg,POINTER_GUARD(TPREG); \
	xor	destreg,tmpreg,reg
#  define PTR_DEMANGLE(reg, tmpreg) PTR_MANGLE (reg, tmpreg)
#  define PTR_DEMANGLE2(reg, tmpreg) PTR_MANGLE2 (reg, tmpreg)
#  define PTR_DEMANGLE3(destreg, reg, tmpreg) PTR_MANGLE3 (destreg, reg, tmpreg)
# else
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ THREAD_GET_POINTER_GUARD ())
#  define PTR_DEMANGLE(var)	PTR_MANGLE (var)
# endif
#endif

/* List of system calls which are supported as vsyscalls.  */
#define VDSO_NAME  "LINUX_2.6.15"
#define VDSO_HASH  123718565

#if defined(__PPC64__) || defined(__powerpc64__)
#define HAVE_CLOCK_GETRES64_VSYSCALL	"__kernel_clock_getres"
#define HAVE_CLOCK_GETTIME64_VSYSCALL	"__kernel_clock_gettime"
#else
#define HAVE_CLOCK_GETRES_VSYSCALL	"__kernel_clock_getres"
#define HAVE_CLOCK_GETTIME_VSYSCALL	"__kernel_clock_gettime"
#endif
#define HAVE_GETCPU_VSYSCALL		"__kernel_getcpu"
#define HAVE_TIME_VSYSCALL		"__kernel_time"
#define HAVE_GETTIMEOFDAY_VSYSCALL      "__kernel_gettimeofday"
#define HAVE_GET_TBFREQ                 "__kernel_get_tbfreq"

#if defined(__PPC64__) || defined(__powerpc64__)
# define HAVE_SIGTRAMP_RT64		"__kernel_sigtramp_rt64"
#else
# define HAVE_SIGTRAMP_32		"__kernel_sigtramp32"
# define HAVE_SIGTRAMP_RT32		"__kernel_sigtramp_rt32"
#endif

#endif /* _LINUX_POWERPC_SYSDEP_H  */
