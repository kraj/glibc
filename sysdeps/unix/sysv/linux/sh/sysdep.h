/* Copyright (C) 1992-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>, August 1995.
   Changed by Kaz Kojima, <kkojima@rr.iij4u.or.jp>.

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

#ifndef _LINUX_SH_SYSDEP_H
#define _LINUX_SH_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/unix/sh/sysdep.h>

#ifdef __ASSEMBLER__
# ifndef PIC
#  define SYSCALL_ERROR_HANDLER	\
	mov.l 0f,r1; \
	jmp @r1; \
	 mov r0,r4; \
	.align 2; \
     0: .long __syscall_error
#  else
#   define SYSCALL_ERROR_HANDLER \
	neg r0,r1; \
	mov r12,r2; \
	cfi_register (r12, r2); \
	mov.l 0f,r12; \
	mova 0f,r0; \
	add r0,r12; \
	mov.l 1f,r0; \
	stc gbr, r4; \
	mov.l @(r0,r12),r0; \
	mov r2,r12; \
	cfi_restore (r12); \
	add r4,r0; \
	mov.l r1,@r0; \
	bra .Lpseudo_end; \
	 mov #-1,r0; \
	.align 2; \
     0: .long _GLOBAL_OFFSET_TABLE_; \
     1: .long __libc_errno@GOTTPOFF
# endif	/* PIC */

# ifdef NEED_SYSCALL_INST_PAD
#  define SYSCALL_INST_PAD \
	or r0,r0; or r0,r0; or r0,r0; or r0,r0; or r0,r0
# else
#  define SYSCALL_INST_PAD
# endif

#else /* not __ASSEMBLER__ */

# ifdef NEED_SYSCALL_INST_PAD
#  define SYSCALL_INST_PAD "\
	or r0,r0; or r0,r0; or r0,r0; or r0,r0; or r0,r0"
# else
#  define SYSCALL_INST_PAD
# endif

static inline long int
__internal_syscall0 (long int name)
{
  unsigned long int resultvar;
  register long int r3 asm ("%r3") = name;
  asm volatile ("trapa #0x10\n\t" SYSCALL_INST_PAD
		: "=z" (resultvar)
		: "r" (r3)
		: "memory", "t");
  return resultvar;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  unsigned long int resultvar;
  register long int r3 asm ("%r3") = name;
  register long int r4 asm ("%r4") = arg1;
  asm volatile ("trapa #0x11\n\t"
		SYSCALL_INST_PAD
		: "=z" (resultvar)
		: "r" (r3), "r" (r4)
		: "memory", "t");
  return resultvar;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  unsigned long int resultvar;
  register long int r3 asm ("%r3") = name;
  register long int r4 asm ("%r4") = arg1;
  register long int r5 asm ("%r5") = arg2;
  asm volatile ("trapa #0x12\n\t"
		SYSCALL_INST_PAD
		: "=z" (resultvar)
		: "r" (r3), "r" (r4), "r" (r5)
		: "memory", "t");
  return resultvar;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  unsigned long int resultvar;
  register long int r3 asm ("%r3") = name;
  register long int r4 asm ("%r4") = arg1;
  register long int r5 asm ("%r5") = arg2;
  register long int r6 asm ("%r6") = arg3;
  asm volatile ("trapa #0x13\n\t"
		SYSCALL_INST_PAD
		: "=z" (resultvar)
		: "r" (r3), "r" (r4), "r" (r5), "r" (r6)
		: "memory", "t");
  return resultvar;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  unsigned long int resultvar;
  register long int r3 asm ("%r3") = name;
  register long int r4 asm ("%r4") = arg1;
  register long int r5 asm ("%r5") = arg2;
  register long int r6 asm ("%r6") = arg3;
  register long int r7 asm ("%r7") = arg4;
  asm volatile ("trapa #0x14\n\t"
		 SYSCALL_INST_PAD
		: "=z" (resultvar)
		: "r" (r3), "r" (r4), "r" (r5), "r" (r6), "r" (r7)
		: "memory", "t");
  return resultvar;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  unsigned long int resultvar;
  register long int r3 asm ("%r3") = name;
  register long int r4 asm ("%r4") = arg1;
  register long int r5 asm ("%r5") = arg2;
  register long int r6 asm ("%r6") = arg3;
  register long int r7 asm ("%r7") = arg4;
  register long int r0 asm ("%r0") = arg5;
  asm volatile ("mov.l @(0,r15),r0\n\t"
		"trapa #0x15\n\t"
		SYSCALL_INST_PAD
		: "=z" (resultvar)
		: "r" (r3), "r" (r4), "r" (r5), "r" (r6), "r" (r7), "0" (r0)
		: "memory", "t");
  return resultvar;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  unsigned long int resultvar;
  register long int r3 asm ("%r3") = name;
  register long int r4 asm ("%r4") = arg1;
  register long int r5 asm ("%r5") = arg2;
  register long int r6 asm ("%r6") = arg3;
  register long int r7 asm ("%r7") = arg4;
  register long int r0 asm ("%r0") = arg5;
  register long int r1 asm ("%r1") = arg6;
  asm volatile ("mov.l @(0,r15),r0\n\t"
		"mov.l @(4,r15),r1\n\t"
		"trapa #0x16\n\t"
		SYSCALL_INST_PAD
		: "=z" (resultvar)
		: "r" (r3), "r" (r4), "r" (r5), "r" (r6), "r" (r7), "0" (r0),
		  "r" (r1)
		: "memory", "t");
  return resultvar;
}

#endif	/* __ASSEMBLER__ */

/* Pointer mangling support.  */
#if IS_IN (rtld)
/* We cannot use the thread descriptor because in ld.so we use setjmp
   earlier than the descriptor is initialized.  Using a global variable
   is too complicated here since we have no PC-relative addressing mode.  */
#else
# ifdef __ASSEMBLER__
#  include <tcb-offsets.h>
#  define PTR_MANGLE(reg, tmp) \
     stc gbr,tmp; mov.l @(POINTER_GUARD,tmp),tmp; xor tmp,reg
#  define PTR_MANGLE2(reg, tmp)	xor tmp,reg
#  define PTR_DEMANGLE(reg, tmp)	PTR_MANGLE (reg, tmp)
#  define PTR_DEMANGLE2(reg, tmp)	PTR_MANGLE2 (reg, tmp)
# else
#  define PTR_MANGLE(var) \
     (var) = (void *) ((uintptr_t) (var) ^ THREAD_GET_POINTER_GUARD ())
#  define PTR_DEMANGLE(var)	PTR_MANGLE (var)
# endif
#endif

#endif /* linux/sh/sysdep.h */
