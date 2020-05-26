/* Copyright (C) 2000-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2000.

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

#ifndef _LINUX_SPARC_SYSDEP_H
#define _LINUX_SPARC_SYSDEP_H 1

#include <sysdeps/unix/sysdep.h>
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/sparc/sysdep.h>

#ifdef __ASSEMBLER__

#define	ret		retl; nop
#define	ret_NOERRNO	retl; nop
#define	ret_ERRVAL	retl; nop
#define	r0		%o0
#define	r1		%o1
#define	MOVE(x,y)	mov x, y

#else	/* __ASSEMBLER__ */

# define VDSO_NAME  "LINUX_2.6"
# define VDSO_HASH  61765110

/* List of system calls which are supported as vsyscalls.  */
# ifdef __arch64__
#  define HAVE_CLOCK_GETTIME64_VSYSCALL	"__vdso_clock_gettime"
# else
#  define HAVE_CLOCK_GETTIME_VSYSCALL	"__vdso_clock_gettime"
# endif
# define HAVE_GETTIMEOFDAY_VSYSCALL	"__vdso_gettimeofday"

#ifdef __arch64__
# define __SYSCALL_TRAP_NUM     "0x6d"
# define __SYSCALL_BCC_INST     "bcc,pt %%xcc, 1f"
# define __SYSCALL_ASM_CLOBBERS						\
	__SYSCALL_COMMON_CLOBBERS ,					\
	"f32", "f34", "f36", "f38", "f40", "f42", "f44", "f46",		\
	"f48", "f50", "f52", "f54", "f56", "f58", "f60", "f62"
#else
# define __SYSCALL_TRAP_NUM     "0x10"
# define __SYSCALL_BCC_INST     "bcc    1f"
# define __SYSCALL_ASM_CLOBBERS						\
 	__SYSCALL_COMMON_CLOBBERS
#endif /* __arch64__  */
#define __SYSCALL_COMMON_CLOBBERS					\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",			\
	"f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",		\
	"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",		\
	"f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",		\
	"cc", "memory"

static inline long int
__internal_syscall0 (long int name)
{
  register long int g1 asm ("g1") = name;
  register long int o0 asm ("o0");
  asm volatile ("ta " __SYSCALL_TRAP_NUM ";"
		__SYSCALL_BCC_INST ";"
		" nop;"
		"sub  %%g0, %%o0, %%o0;"
		"1:"
		: "=r" (o0)
		: "r" (g1)
		: __SYSCALL_ASM_CLOBBERS);
  return o0;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  register long int g1 asm ("g1") = name;
  register long int o0 asm ("o0") = arg1;
  asm volatile ("ta " __SYSCALL_TRAP_NUM ";"
		__SYSCALL_BCC_INST ";"
		" nop;"
		"sub  %%g0, %%o0, %%o0;"
		"1:"
		: "=r" (o0)
		: "r" (g1), "0" (o0)
		: __SYSCALL_ASM_CLOBBERS);
  return o0;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  register long int g1 asm ("g1") = name;
  register long int o0 asm ("o0") = arg1;
  register long int o1 asm ("o1") = arg2;
  asm volatile ("ta " __SYSCALL_TRAP_NUM ";"
		__SYSCALL_BCC_INST ";"
		" nop;"
		"sub  %%g0, %%o0, %%o0;"
		"1:"
		: "=r" (o0)
		: "r" (g1), "0" (o0), "r" (o1)
		: __SYSCALL_ASM_CLOBBERS);
  return o0;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register long int g1 asm ("g1") = name;
  register long int o0 asm ("o0") = arg1;
  register long int o1 asm ("o1") = arg2;
  register long int o2 asm ("o2") = arg3;
  asm volatile ("ta " __SYSCALL_TRAP_NUM ";"
		__SYSCALL_BCC_INST ";"
		" nop;"
		"sub  %%g0, %%o0, %%o0;"
		"1:"
		: "=r" (o0)
		: "r" (g1), "0" (o0), "r" (o1), "r" (o2)
		: __SYSCALL_ASM_CLOBBERS);
  return o0;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  register long int g1 asm ("g1") = name;
  register long int o0 asm ("o0") = arg1;
  register long int o1 asm ("o1") = arg2;
  register long int o2 asm ("o2") = arg3;
  register long int o3 asm ("o3") = arg4;
  asm volatile ("ta " __SYSCALL_TRAP_NUM ";"
		__SYSCALL_BCC_INST ";"
		" nop;"
		"sub  %%g0, %%o0, %%o0;"
		"1:"
		: "=r" (o0)
		: "r" (g1), "0" (o0), "r" (o1), "r" (o2), "r" (o3)
		: __SYSCALL_ASM_CLOBBERS);
  return o0;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  register long int g1 asm ("g1") = name;
  register long int o0 asm ("o0") = arg1;
  register long int o1 asm ("o1") = arg2;
  register long int o2 asm ("o2") = arg3;
  register long int o3 asm ("o3") = arg4;
  register long int o4 asm ("o4") = arg5;
  asm volatile ("ta " __SYSCALL_TRAP_NUM ";"
		__SYSCALL_BCC_INST ";"
		" nop;"
		"sub  %%g0, %%o0, %%o0;"
		"1:"
		: "=r" (o0)
		: "r" (g1), "0" (o0), "r" (o1), "r" (o2), "r" (o3), "r" (o4)
		: __SYSCALL_ASM_CLOBBERS);
  return o0;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  register long int g1 asm ("g1") = name;
  register long int o0 asm ("o0") = arg1;
  register long int o1 asm ("o1") = arg2;
  register long int o2 asm ("o2") = arg3;
  register long int o3 asm ("o3") = arg4;
  register long int o4 asm ("o4") = arg5;
  register long int o5 asm ("o5") = arg6;
  asm volatile ("ta " __SYSCALL_TRAP_NUM ";"
		__SYSCALL_BCC_INST ";"
		" nop;"
		"sub  %%g0, %%o0, %%o0;"
		"1:"
		: "=r" (o0)
		: "r" (g1), "0" (o0), "r" (o1), "r" (o2), "r" (o3), "r" (o4),
		  "r" (o5)
		: __SYSCALL_ASM_CLOBBERS);
  return o0;
}

#endif	/* __ASSEMBLER__ */

#endif /* _LINUX_SPARC_SYSDEP_H */
