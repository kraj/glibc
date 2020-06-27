/* Copyright (C) 1996-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Andreas Schwab, <schwab@issan.informatik.uni-dortmund.de>,
   December 1995.

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

#include <sysdeps/unix/sysv/linux/sysdep.h>

#ifdef __ASSEMBLER__

/* We don't want the label for the error handler to be visible in the symbol
   table when we define it here.  */
# ifdef PIC
#  define SYSCALL_ERROR_LABEL .Lsyscall_error
# else
#  define SYSCALL_ERROR_LABEL __syscall_error
# endif

# ifdef PIC
#  define SYSCALL_ERROR_HANDLER						      \
SYSCALL_ERROR_LABEL:							      \
    neg.l %d0;								      \
    move.l %d0, -(%sp);							      \
    cfi_adjust_cfa_offset (4);						      \
    jbsr __m68k_read_tp@PLTPC;						      \
    SYSCALL_ERROR_LOAD_GOT (%a1);					      \
    add.l (__libc_errno@TLSIE, %a1), %a0;				      \
    move.l (%sp)+, (%a0);						      \
    cfi_adjust_cfa_offset (-4);						      \
    move.l &-1, %d0;							      \
    /* Copy return value to %a0 for syscalls that are declared to return      \
       a pointer (e.g., mmap).  */					      \
    move.l %d0, %a0;							      \
    rts;
# else
#  define SYSCALL_ERROR_HANDLER	/* Nothing here; code in sysdep.S is used.  */
# endif /* PIC */

/* Linux takes system call arguments in registers:

	syscall number	%d0	     call-clobbered
	arg 1		%d1	     call-clobbered
	arg 2		%d2	     call-saved
	arg 3		%d3	     call-saved
	arg 4		%d4	     call-saved
	arg 5		%d5	     call-saved
	arg 6		%a0	     call-clobbered

   The stack layout upon entering the function is:

	24(%sp)		Arg# 6
	20(%sp)		Arg# 5
	16(%sp)		Arg# 4
	12(%sp)		Arg# 3
	 8(%sp)		Arg# 2
	 4(%sp)		Arg# 1
	  (%sp)		Return address

   (Of course a function with say 3 arguments does not have entries for
   arguments 4 and 5.)

   Separate move's are faster than movem, but need more space.  Since
   speed is more important, we don't use movem.  Since %a0 and %a1 are
   scratch registers, we can use them for saving as well.  */

# else /* not __ASSEMBLER__ */

static inline long int
__internal_syscall0 (long int name)
{
  register int d0 asm ("d0") = name;
  asm volatile ("trap #0"
		: "=d" (d0)
		: "0" (d0)
		: "memory");
  return d0;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  register int d1 asm ("d1") = arg1;
  register int d0 asm ("d0") = name;
  asm volatile ("trap #0"
		: "=d" (d0)
		: "0" (d0), "d" (d1)
		: "memory");
  return d0;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  register int d1 asm ("d1") = arg1;
  register int d2 asm ("d2") = arg2;
  register int d0 asm ("d0") = name;
  asm volatile ("trap #0"
		: "=d" (d0)
		: "0" (d0), "d" (d1), "d" (d2)
		: "memory");
  return d0;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register int d1 asm ("d1") = arg1;
  register int d2 asm ("d2") = arg2;
  register int d3 asm ("d3") = arg3;
  register int d0 asm ("d0") = name;
  asm volatile ("trap #0"
		: "=d" (d0)
		: "0" (d0), "d" (d1), "d" (d2), "d" (d3)
		: "memory");
  return d0;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  register int d1 asm ("d1") = arg1;
  register int d2 asm ("d2") = arg2;
  register int d3 asm ("d3") = arg3;
  register int d4 asm ("d4") = arg4;
  register int d0 asm ("d0") = name;
  asm volatile ("trap #0"
		: "=d" (d0)
		: "0" (d0), "d" (d1), "d" (d2), "d" (d3), "d" (d4)
		: "memory");
  return d0;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  register int d1 asm ("d1") = arg1;
  register int d2 asm ("d2") = arg2;
  register int d3 asm ("d3") = arg3;
  register int d4 asm ("d4") = arg4;
  register int d5 asm ("d5") = arg5;
  register int d0 asm ("d0") = name;
  asm volatile ("trap #0"
		: "=d" (d0)
		: "0" (d0), "d" (d1), "d" (d2), "d" (d3), "d" (d4), "d" (d5)
		: "memory");
  return d0;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  register int d1 asm ("d1") = arg1;
  register int d2 asm ("d2") = arg2;
  register int d3 asm ("d3") = arg3;
  register int d4 asm ("d4") = arg4;
  register int d5 asm ("d5") = arg5;
  register int a0 asm ("a0") = arg6;
  register int d0 asm ("d0") = name;
  asm volatile ("trap #0"
		: "=d" (d0)
		: "0" (d0), "d" (d1), "d" (d2), "d" (d3), "d" (d4), "d" (d5),
		  "a" (a0)
		: "memory");
  return d0;
}

#undef HAVE_INTERNAL_BRK_ADDR_SYMBOL
#define HAVE_INTERNAL_BRK_ADDR_SYMBOL 1

#endif /* not __ASSEMBLER__ */

/* Pointer mangling is not yet supported for M68K.  */
#define PTR_MANGLE(var) (void) (var)
#define PTR_DEMANGLE(var) (void) (var)

#if defined NEED_DL_SYSINFO || defined NEED_DL_SYSINFO_DSO
/* M68K needs system-supplied DSO to access TLS helpers
   even when statically linked.  */
# define NEED_STATIC_SYSINFO_DSO 1
#endif
