/* Copyright (C) 1992-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.org>, August 1995.

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

#ifndef _LINUX_I386_SYSDEP_H
#define _LINUX_I386_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/unix/sysdep.h>
#include <sysdeps/i386/sysdep.h>
#include <dl-sysdep.h>
#include <tcb-offsets.h>


/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)	__NR_##syscall_name

#ifndef I386_USE_SYSENTER
# if defined USE_DL_SYSINFO \
     && (IS_IN (libc) || IS_IN (libpthread))
#  define I386_USE_SYSENTER	1
# else
#  define I386_USE_SYSENTER	0
# endif
#endif

/* Since GCC 5 and above can properly spill %ebx with PIC when needed,
   we can inline syscalls with 6 arguments if GCC 5 or above is used
   to compile glibc.  Disable GCC 5 optimization when compiling for
   profiling or when -fno-omit-frame-pointer is used since asm ("ebp")
   can't be used to put the 6th argument in %ebp for syscall.  */
#if !defined PROF && CAN_USE_REGISTER_ASM_EBP
# define OPTIMIZE_FOR_GCC_5
#endif

/* Linux takes system call arguments in registers:

	syscall number	%eax	     call-clobbered
	arg 1		%ebx	     call-saved
	arg 2		%ecx	     call-clobbered
	arg 3		%edx	     call-clobbered
	arg 4		%esi	     call-saved
	arg 5		%edi	     call-saved
	arg 6		%ebp	     call-saved

   The stack layout upon entering the function is:

	24(%esp)	Arg# 6
	20(%esp)	Arg# 5
	16(%esp)	Arg# 4
	12(%esp)	Arg# 3
	 8(%esp)	Arg# 2
	 4(%esp)	Arg# 1
	  (%esp)	Return address

   (Of course a function with say 3 arguments does not have entries for
   arguments 4, 5, and 6.)

   The calling conventions for Linux tell that among the registers used
   for parameters %ecx and %edx need not be saved.  Beside this we may
   clobber this registers even when they are not used for parameter
   passing.  */

#ifdef __ASSEMBLER__
/* The original calling convention for system calls on Linux/i386 is
   to use int $0x80.  */
#if I386_USE_SYSENTER
# ifdef PIC
#  define ENTER_KERNEL call *%gs:SYSINFO_OFFSET
# else
#  define ENTER_KERNEL call *_dl_sysinfo
# endif
#else
# define ENTER_KERNEL int $0x80
#endif

#else
extern int __syscall_error (int)
  attribute_hidden __attribute__ ((__regparm__ (1)));

#ifndef OPTIMIZE_FOR_GCC_5
/* We need some help from the assembler to generate optimal code.  We
   define some macros here which later will be used.  */
asm (".L__X'%ebx = 1\n\t"
     ".L__X'%ecx = 2\n\t"
     ".L__X'%edx = 2\n\t"
     ".L__X'%eax = 3\n\t"
     ".L__X'%esi = 3\n\t"
     ".L__X'%edi = 3\n\t"
     ".L__X'%ebp = 3\n\t"
     ".L__X'%esp = 3\n\t"
     ".macro bpushl name reg\n\t"
     ".if 1 - \\name\n\t"
     ".if 2 - \\name\n\t"
     "error\n\t"
     ".else\n\t"
     "xchgl \\reg, %ebx\n\t"
     ".endif\n\t"
     ".endif\n\t"
     ".endm\n\t"
     ".macro bpopl name reg\n\t"
     ".if 1 - \\name\n\t"
     ".if 2 - \\name\n\t"
     "error\n\t"
     ".else\n\t"
     "xchgl \\reg, %ebx\n\t"
     ".endif\n\t"
     ".endif\n\t"
     ".endm\n\t");

/* Six-argument syscalls use an out-of-line helper, because an inline
   asm using all registers apart from %esp cannot work reliably and
   the assembler does not support describing an asm that saves and
   restores %ebp itself as a separate stack frame.  This structure
   stores the arguments not passed in registers; %edi is passed with a
   pointer to this structure.  */
struct libc_do_syscall_args
{
  int ebx, edi, ebp;
};
#endif

# define VDSO_NAME  "LINUX_2.6"
# define VDSO_HASH  61765110

/* List of system calls which are supported as vsyscalls.  */
# define HAVE_CLOCK_GETTIME_VSYSCALL    "__vdso_clock_gettime"
# define HAVE_CLOCK_GETTIME64_VSYSCALL  "__vdso_clock_gettime64"
# define HAVE_GETTIMEOFDAY_VSYSCALL     "__vdso_gettimeofday"
# define HAVE_TIME_VSYSCALL             "__vdso_time"
# define HAVE_CLOCK_GETRES_VSYSCALL     "__vdso_clock_getres"

#if I386_USE_SYSENTER
# ifdef PIC
#  define SYSCALL_INSNS "call *%%gs:%P2\n\t"
#  define SYSCALL_CONST , "i" (SYSINFO_OFFSET)
# else
#  define SYSCALL_INSNS "call *_dl_sysinfo\n\t"
#  define SYSCALL_CONST
# endif /* PIC  */
#else
# define SYSCALL_INSNS "int $0x80\n\t"
# define SYSCALL_CONST
#endif /* I386_USE_SYSENTER  */

#if defined OPTIMIZE_FOR_GCC_5 || !defined PIC
# define SYSCALL_LOAD_ARGS_1_2
# define SYSCALL_RESTORE_ARGS_1_2
# define SYSCALL_LOAD_ARGS_3_4
# define SYSCALL_RESTORE_ARGS_3_4
# define SYSCALL_LOAD_ARGS_5
# define SYSCALL_RESTORE_ARGS_5
# define SYSCALL_ASMFMT_1(__arg1) \
       , "b" (__arg1)
# define SYSCALL_ASMFMT_2(__arg1, __arg2) \
       , "b" (__arg1), "c" (__arg2)
# define SYSCALL_ASMFMT_3(__arg1, __arg2, __arg3) \
       , "b" (__arg1), "c" (__arg2), "d" (__arg3)
# define SYSCALL_ASMFMT_4(__arg1, __arg2, __arg3, __arg4) \
       , "b" (__arg1), "c" (__arg2), "d" (__arg3), "S" (__arg4)
# define SYSCALL_ASMFMT_5(__arg1, __arg2, __arg3, __arg4, __arg5) \
        , "b" (__arg1), "c" (__arg2), "d" (__arg3), "S" (__arg4), "D" (__arg5)
#else
# if I386_USE_SYSENTER
#  define SYSCALL_LOAD_ARGS_1_2     "bpushl .L__X'%k3, %k3\n\t"
#  define SYSCALL_RESTORE_ARGS_1_2  "bpopl .L__X'%k3, %k3\n\t"
#  define SYSCALL_LOAD_ARGS_3_4     "xchgl %%ebx, %%edi\n\t"
#  define SYSCALL_RESTORE_ARGS_3_4  "xchgl %%edi, %%ebx\n\t"
#  define SYSCALL_LOAD_ARGS_5       "movl %%ebx, %4\n\t" \
				    "movl %3, %%ebx\n\t"
#  define SYSCALL_RESTORE_ARGS_5    "movl %4, %%ebx"
# else
#  define SYSCALL_LOAD_ARGS_1_2     "bpushl .L__X'%k2, %k2\n\t"
#  define SYSCALL_RESTORE_ARGS_1_2  "bpopl .L__X'%k2, %k2\n\t"
#  define SYSCALL_LOAD_ARGS_3_4     "xchgl %%ebx, %%edi\n\t"
#  define SYSCALL_RESTORE_ARGS_3_4  "xchgl %%edi, %%ebx\n\t"
#  define SYSCALL_LOAD_ARGS_5       "movl %%ebx, %3\n\t" \
				    "movl %2, %%ebx\n\t"
#  define SYSCALL_RESTORE_ARGS_5    "movl %3, %%ebx"
# endif /* I386_USE_SYSENTER  */
# define SYSCALL_ASMFMT_1(__arg1) \
        , "cd" (__arg1)
# define SYSCALL_ASMFMT_2(__arg1, __arg2) \
        , "d" (__arg1), "c" (__arg2)
# define SYSCALL_ASMFMT_3(__arg1, __arg2, __arg3) \
        , "D" (__arg1), "c" (__arg2), "d" (__arg3)
# define SYSCALL_ASMFMT_4(__arg1, __arg2, __arg3, __arg4) \
        , "D" (__arg1), "c" (__arg2), "d" (__arg3), "S" (__arg4)
# define SYSCALL_ASMFMT_5(__arg1, __arg2, __arg3, __arg4, __arg5) \
        , "g" (__arg1), "m" ((long int){0}), "c" (__arg2), "d" (__arg3), \
	  "S" (__arg4), "D" (__arg5)
#endif /* OPTIMIZE_FOR_GCC_5 || !defined PIC  */

static inline long int
__internal_syscall0 (long int name)
{
  unsigned long int resultvar;
  asm volatile (SYSCALL_INSNS
		: "=a" (resultvar)
		: "a" (name) SYSCALL_CONST
		: "memory", "cc");
  return resultvar;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  unsigned long int resultvar;
  asm volatile (SYSCALL_LOAD_ARGS_1_2
		"movl %1, %%eax\n\t"
		SYSCALL_INSNS
		SYSCALL_RESTORE_ARGS_1_2
		: "=a" (resultvar)
		: "a" (name) SYSCALL_CONST
		  SYSCALL_ASMFMT_1 (arg1)
		: "memory", "cc");
  return resultvar;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1, __syscall_arg_t arg2)
{
  unsigned long int resultvar;
  asm volatile (SYSCALL_LOAD_ARGS_1_2
		"movl %1, %%eax\n\t"
		SYSCALL_INSNS
		SYSCALL_RESTORE_ARGS_1_2
		: "=a" (resultvar)
		: "a" (name) SYSCALL_CONST
		  SYSCALL_ASMFMT_2 (arg1, arg2)
		: "memory", "cc");
  return resultvar;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1, __syscall_arg_t arg2,
		   __syscall_arg_t arg3)
{
  unsigned long int resultvar;
  asm volatile (SYSCALL_LOAD_ARGS_3_4
		"movl %1, %%eax\n\t"
		SYSCALL_INSNS
		SYSCALL_RESTORE_ARGS_3_4
		: "=a" (resultvar)
		: "a" (name) SYSCALL_CONST
		  SYSCALL_ASMFMT_3 (arg1, arg2, arg3)
		: "memory", "cc");
  return resultvar;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1, __syscall_arg_t arg2,
		   __syscall_arg_t arg3, __syscall_arg_t arg4)
{
  unsigned long int resultvar;
  asm volatile (SYSCALL_LOAD_ARGS_3_4
		"movl %1, %%eax\n\t"
		SYSCALL_INSNS
		SYSCALL_RESTORE_ARGS_3_4
		: "=a" (resultvar)
		: "a" (name) SYSCALL_CONST
		  SYSCALL_ASMFMT_4 (arg1, arg2, arg3, arg4)
		: "memory", "cc");
  return resultvar;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1, __syscall_arg_t arg2,
		   __syscall_arg_t arg3, __syscall_arg_t arg4,
		   __syscall_arg_t arg5)
{
  unsigned long int resultvar;
  asm volatile (SYSCALL_LOAD_ARGS_5
		"movl %1, %%eax\n\t"
		SYSCALL_INSNS
		SYSCALL_RESTORE_ARGS_5
		: "=a" (resultvar)
		: "a" (name) SYSCALL_CONST
		  SYSCALL_ASMFMT_5 (arg1, arg2, arg3, arg4, arg5)
		: "memory", "cc");
  return resultvar;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1, __syscall_arg_t arg2,
		   __syscall_arg_t arg3, __syscall_arg_t arg4,
		   __syscall_arg_t arg5, __syscall_arg_t arg6)
{
  unsigned long int resultvar;
# ifdef OPTIMIZE_FOR_GCC_5
  register unsigned long int a6 asm ("ebp") = arg6;
  asm volatile (SYSCALL_INSNS
		: "=a" (resultvar)
		: "a" (name) SYSCALL_CONST, "b" (arg1), "c" (arg2),
		  "d" (arg3), "S" (arg4), "D" (arg5), "r" (a6)
		: "memory", "cc");
# else
  /* Six-argument syscalls use an out-of-line helper, because an inline
     asm using all registers apart from %esp cannot work reliably and
     the assembler does not support describing an asm that saves and
     restores %ebp itself as a separate stack frame.  This structure
     stores the arguments not passed in registers; %edi is passed with a
     pointer to this structure.  */
  __syscall_arg_t _xv[] = { arg1, arg5, arg6 };
  asm volatile ("movl %1, %%eax\n\t"
		"call __libc_do_syscall"
		: "=a" (resultvar)
		: "a" (name), "c" (arg2), "d" (arg3), "S" (arg4), "D" (&_xv)
		: "memory", "cc");
# endif /* OPTIMIZE_FOR_GCC_5  */
  return resultvar;
}

#undef HAVE_INTERNAL_BRK_ADDR_SYMBOL
#define HAVE_INTERNAL_BRK_ADDR_SYMBOL 1

#endif	/* __ASSEMBLER__ */


/* Pointer mangling support.  */
#if IS_IN (rtld)
/* We cannot use the thread descriptor because in ld.so we use setjmp
   earlier than the descriptor is initialized.  Using a global variable
   is too complicated here since we have no PC-relative addressing mode.  */
#else
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(reg)	xorl %gs:POINTER_GUARD, reg;		      \
				roll $9, reg
#  define PTR_DEMANGLE(reg)	rorl $9, reg;				      \
				xorl %gs:POINTER_GUARD, reg
# else
#  define PTR_MANGLE(var)	asm ("xorl %%gs:%c2, %0\n"		      \
				     "roll $9, %0"			      \
				     : "=r" (var)			      \
				     : "0" (var),			      \
				       "i" (offsetof (tcbhead_t,	      \
						      pointer_guard)))
#  define PTR_DEMANGLE(var)	asm ("rorl $9, %0\n"			      \
				     "xorl %%gs:%c2, %0"		      \
				     : "=r" (var)			      \
				     : "0" (var),			      \
				       "i" (offsetof (tcbhead_t,	      \
						      pointer_guard)))
# endif
#endif

/* Each shadow stack slot takes 4 bytes.  Assuming that each stack
   frame takes 128 bytes, this is used to compute shadow stack size
   from stack size.  */
#define STACK_SIZE_TO_SHADOW_STACK_SIZE_SHIFT 5

#endif /* linux/i386/sysdep.h */
