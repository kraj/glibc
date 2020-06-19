/* Copyright (C) 2001-2020 Free Software Foundation, Inc.
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

#ifndef _LINUX_X86_64_SYSDEP_H
#define _LINUX_X86_64_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/x86_64/sysdep.h>
#include <sysdeps/unix/sysdep.h>
#include <tcb-offsets.h>

#ifndef __ASSEMBLER__
/* The Linux/x86-64 kernel expects the system call parameters in
   registers according to the following table:

    syscall number	rax
    arg 1		rdi
    arg 2		rsi
    arg 3		rdx
    arg 4		r10
    arg 5		r8
    arg 6		r9

    The Linux kernel uses and destroys internally these registers:
    return address from
    syscall		rcx
    eflags from syscall	r11

    Normal function call, including calls to the system call stub
    functions in the libc, get the first six parameters passed in
    registers and the seventh parameter and later on the stack.  The
    register use is as follows:

     arg 1		rdi
     arg 2		rsi
     arg 3		rdx
     arg 4		rcx
     arg 5		r8
     arg 6		r9

    We have to take care that the stack is aligned to 16 bytes.  When
    called the stack is not aligned since the return address has just
    been pushed.

    Syscalls of more than 6 arguments are not supported.  */


/* NB: This also works when X is an array.  For an array X,  type of
   (X) - (X) is ptrdiff_t, which is signed, since size of ptrdiff_t
   == size of pointer, cast is a NOP.   */
#ifndef ARGIFY
/* Explicit cast the argument.  */
# define ARGIFY(X) ((TYPEFY1 (X)) (X))
#endif

static inline long int
__internal_syscall0 (long int name)
{
  unsigned long int resultvar;
  asm volatile ("syscall\n\t"
		: "=a" (resultvar)
		: "0" (name)
		: "memory", "cc", "r11", "cx");
  return resultvar;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  unsigned long int resultvar;
  register __syscall_arg_t a1 asm ("rdi") = arg1;
  asm volatile ("syscall\n\t"
		: "=a" (resultvar)
		: "0" (name),  "r" (a1)
		: "memory", "cc", "r11", "cx");
  return resultvar;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  unsigned long int resultvar;
  register __syscall_arg_t a1 asm ("rdi") = arg1;
  register __syscall_arg_t a2 asm ("rsi") = arg2;
  asm volatile ("syscall\n\t"
		: "=a" (resultvar)
		: "0" (name),  "r" (a1), "r" (a2)
		: "memory", "cc", "r11", "cx");
  return resultvar;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  unsigned long int resultvar;
  register __syscall_arg_t a1 asm ("rdi") = arg1;
  register __syscall_arg_t a2 asm ("rsi") = arg2;
  register __syscall_arg_t a3 asm ("rdx") = arg3;
  asm volatile ("syscall\n\t"
		: "=a" (resultvar)
		: "0" (name),  "r" (a1), "r" (a2), "r" (a3)
		: "memory", "cc", "r11", "cx");
  return resultvar;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  unsigned long int resultvar;
  register __syscall_arg_t a1 asm ("rdi") = arg1;
  register __syscall_arg_t a2 asm ("rsi") = arg2;
  register __syscall_arg_t a3 asm ("rdx") = arg3;
  register __syscall_arg_t a4 asm ("r10") = arg4;
  asm volatile ("syscall\n\t"
		: "=a" (resultvar)
		: "0" (name),  "r" (a1), "r" (a2), "r" (a3), "r" (a4)
		: "memory", "cc", "r11", "cx");
  return resultvar;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  unsigned long int resultvar;
  register __syscall_arg_t a1 asm ("rdi") = arg1;
  register __syscall_arg_t a2 asm ("rsi") = arg2;
  register __syscall_arg_t a3 asm ("rdx") = arg3;
  register __syscall_arg_t a4 asm ("r10") = arg4;
  register __syscall_arg_t a5 asm ("r8") = arg5;
  asm volatile ("syscall\n\t"
		: "=a" (resultvar)
		: "0" (name),  "r" (a1), "r" (a2), "r" (a3), "r" (a4),
		  "r" (a5)
		: "memory", "cc", "r11", "cx");
  return resultvar;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  unsigned long int resultvar;
  register __syscall_arg_t a1 asm ("rdi") = arg1;
  register __syscall_arg_t a2 asm ("rsi") = arg2;
  register __syscall_arg_t a3 asm ("rdx") = arg3;
  register __syscall_arg_t a4 asm ("r10") = arg4;
  register __syscall_arg_t a5 asm ("r8") = arg5;
  register __syscall_arg_t a6 asm ("r9") = arg6;
  asm volatile ("syscall\n\t"
		: "=a" (resultvar)
		: "0" (name),  "r" (a1), "r" (a2), "r" (a3), "r" (a4),
		  "r" (a5), "r" (a6)
		: "memory", "cc", "r11", "cx");
  return resultvar;
}

# define VDSO_NAME  "LINUX_2.6"
# define VDSO_HASH  61765110

/* List of system calls which are supported as vsyscalls.  */
# define HAVE_CLOCK_GETTIME64_VSYSCALL  "__vdso_clock_gettime"
# define HAVE_GETTIMEOFDAY_VSYSCALL     "__vdso_gettimeofday"
# define HAVE_TIME_VSYSCALL             "__vdso_time"
# define HAVE_GETCPU_VSYSCALL		"__vdso_getcpu"
# define HAVE_CLOCK_GETRES64_VSYSCALL   "__vdso_clock_getres"

# define SINGLE_THREAD_BY_GLOBAL		1

#endif	/* __ASSEMBLER__ */


/* Pointer mangling support.  */
#if IS_IN (rtld)
/* We cannot use the thread descriptor because in ld.so we use setjmp
   earlier than the descriptor is initialized.  */
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(reg)	xor __pointer_chk_guard_local(%rip), reg;    \
				rol $2*LP_SIZE+1, reg
#  define PTR_DEMANGLE(reg)	ror $2*LP_SIZE+1, reg;			     \
				xor __pointer_chk_guard_local(%rip), reg
# else
#  define PTR_MANGLE(reg)	asm ("xor __pointer_chk_guard_local(%%rip), %0\n" \
				     "rol $2*" LP_SIZE "+1, %0"			  \
				     : "=r" (reg) : "0" (reg))
#  define PTR_DEMANGLE(reg)	asm ("ror $2*" LP_SIZE "+1, %0\n"		  \
				     "xor __pointer_chk_guard_local(%%rip), %0"   \
				     : "=r" (reg) : "0" (reg))
# endif
#else
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(reg)	xor %fs:POINTER_GUARD, reg;		      \
				rol $2*LP_SIZE+1, reg
#  define PTR_DEMANGLE(reg)	ror $2*LP_SIZE+1, reg;			      \
				xor %fs:POINTER_GUARD, reg
# else
#  define PTR_MANGLE(var)	asm ("xor %%fs:%c2, %0\n"		      \
				     "rol $2*" LP_SIZE "+1, %0"		      \
				     : "=r" (var)			      \
				     : "0" (var),			      \
				       "i" (offsetof (tcbhead_t,	      \
						      pointer_guard)))
#  define PTR_DEMANGLE(var)	asm ("ror $2*" LP_SIZE "+1, %0\n"	      \
				     "xor %%fs:%c2, %0"			      \
				     : "=r" (var)			      \
				     : "0" (var),			      \
				       "i" (offsetof (tcbhead_t,	      \
						      pointer_guard)))
# endif
#endif

/* How to pass the off{64}_t argument on p{readv,writev}{64}.  */
#undef LO_HI_LONG
#define LO_HI_LONG(val) (val), 0

/* Each shadow stack slot takes 8 bytes.  Assuming that each stack
   frame takes 256 bytes, this is used to compute shadow stack size
   from stack size.  */
#define STACK_SIZE_TO_SHADOW_STACK_SIZE_SHIFT 5

#endif /* linux/x86_64/sysdep.h */
