/* Assembler macros for PA-RISC.
   Copyright (C) 1999-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@cygnus.com>, August 1999.
   Linux/PA-RISC changes by Philipp Rumpf, <prumpf@tux.org>, March 2000.

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

#ifndef _LINUX_HPPA_SYSDEP_H
#define _LINUX_HPPA_SYSDEP_H 1

#include <sysdeps/unix/sysdep.h>
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/hppa/sysdep.h>

#ifndef __ASSEMBLER__

/* Linux takes system call arguments in registers:
	syscall number	gr20
	arg 1		gr26
	arg 2		gr25
	arg 3		gr24
	arg 4		gr23
	arg 5		gr22
	arg 6		gr21

   The compiler calls us by the C convention:
	arg 1		gr26
	arg 2		gr25
	arg 3		gr24
	arg 4		gr23
	arg 5		-52(sp)
	arg 6		-56(sp)

   gr22 and gr21 are caller-saves, so we can just load the arguments
   there and generally be happy. */

/* GCC has to be warned that a syscall may clobber all the ABI
   registers listed as "caller-saves", see page 8, Table 2
   in section 2.2.6 of the PA-RISC RUN-TIME architecture
   document. However! r28 is the result and will conflict with
   the clobber list so it is left out. Also the input arguments
   registers r20 -> r26 will conflict with the list so they
   are treated specially. Although r19 is clobbered by the syscall
   we cannot say this because it would violate ABI, thus we say
   TREG is clobbered and use that register to save/restore r19
   across the syscall. */

/* Inline assembly defines */
#define TREG_ASM "%r4" /* Cant clobber r3, it holds framemarker */
#define SAVE_ASM_PIC	"       copy %%r19, %" TREG_ASM "\n"
#define LOAD_ASM_PIC	"       copy %" TREG_ASM ", %%r19\n"
#define CLOB_TREG	TREG_ASM ,
#define PIC_REG_DEF	register unsigned long __r19 asm("r19");
#define PIC_REG_USE	, "r" (__r19)
#define CALL_CLOB_REGS	"%r1", "%r2", CLOB_TREG \
			"%r20", "%r29", "%r31"

static inline long int
__internal_syscall0 (long int name)
{
  register unsigned long res asm ("r28");
  PIC_REG_DEF
  /* FIXME: HACK save/load r19 around syscall */
  asm volatile (SAVE_ASM_PIC
		"ble  0x100(%%sr2, %%r0)\n"
		"copy %1, %%r20\n"
		LOAD_ASM_PIC
		: "=r" (res)
		: "r" (name) PIC_REG_USE
		: "memory", CALL_CLOB_REGS,
		  "%r21", "%r22", "%r22", "%r23", "%r24", "%r25", "%r26");
  return res;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  register unsigned long res asm ("r28");
  PIC_REG_DEF
  register unsigned long r26 asm ("r26") = arg1;
  /* FIXME: HACK save/load r19 around syscall */
  asm volatile (SAVE_ASM_PIC
		"ble  0x100(%%sr2, %%r0)\n"
		"copy %1, %%r20\n"
		LOAD_ASM_PIC
		: "=r" (res)
		: "r" (name) PIC_REG_USE,
		  "r" (r26)
		: "memory", CALL_CLOB_REGS,
		  "%r21", "%r22", "%r22", "%r23", "%r24", "%r25");
  return res;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  register unsigned long res asm ("r28");
  PIC_REG_DEF
  register unsigned long r26 asm ("r26") = arg1;
  register unsigned long r25 asm ("r25") = arg2;
  /* FIXME: HACK save/load r19 around syscall */
  asm volatile (SAVE_ASM_PIC
		"ble  0x100(%%sr2, %%r0)\n"
		"copy %1, %%r20\n"
		LOAD_ASM_PIC
		: "=r" (res)
		: "r" (name) PIC_REG_USE,
		  "r" (r26), "r" (r25)
		: "memory", CALL_CLOB_REGS,
		  "%r21", "%r22", "%r22", "%r23", "%r24");
  return res;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register unsigned long res asm ("r28");
  PIC_REG_DEF
  register unsigned long r26 asm ("r26") = arg1;
  register unsigned long r25 asm ("r25") = arg2;
  register unsigned long r24 asm ("r24") = arg3;
  /* FIXME: HACK save/load r19 around syscall */
  asm volatile (SAVE_ASM_PIC
		"ble  0x100(%%sr2, %%r0)\n"
		"copy %1, %%r20\n"
		LOAD_ASM_PIC
		: "=r" (res)
		: "r" (name) PIC_REG_USE,
		  "r" (r26), "r" (r25), "r" (r24)
		: "memory", CALL_CLOB_REGS,
		  "%r21", "%r22", "%r22", "%r23");
  return res;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  register unsigned long res asm ("r28");
  PIC_REG_DEF
  register unsigned long r26 asm ("r26") = arg1;
  register unsigned long r25 asm ("r25") = arg2;
  register unsigned long r24 asm ("r24") = arg3;
  register unsigned long r23 asm ("r23") = arg4;
  /* FIXME: HACK save/load r19 around syscall */
  asm volatile (SAVE_ASM_PIC
		"ble  0x100(%%sr2, %%r0)\n"
		"copy %1, %%r20\n"
		LOAD_ASM_PIC
		: "=r" (res)
		: "r" (name) PIC_REG_USE,
		  "r" (r26), "r" (r25), "r" (r24), "r" (r23)
		: "memory", CALL_CLOB_REGS,
		  "%r21", "%r22");
  return res;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  register unsigned long res asm ("r28");
  PIC_REG_DEF
  register unsigned long r26 asm ("r26") = arg1;
  register unsigned long r25 asm ("r25") = arg2;
  register unsigned long r24 asm ("r24") = arg3;
  register unsigned long r23 asm ("r23") = arg4;
  register unsigned long r22 asm ("r22") = arg5;
  /* FIXME: HACK save/load r19 around syscall */
  asm volatile (SAVE_ASM_PIC
		"ble  0x100(%%sr2, %%r0)\n"
		"copy %1, %%r20\n"
		LOAD_ASM_PIC
		: "=r" (res)
		: "r" (name) PIC_REG_USE,
		  "r" (r26), "r" (r25), "r" (r24), "r" (r23), "r" (r22)
		: "memory", CALL_CLOB_REGS,
		  "%r21");
  return res;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  register unsigned long res asm ("r28");
  PIC_REG_DEF
  register unsigned long r26 asm ("r26") = arg1;
  register unsigned long r25 asm ("r25") = arg2;
  register unsigned long r24 asm ("r24") = arg3;
  register unsigned long r23 asm ("r23") = arg4;
  register unsigned long r22 asm ("r22") = arg5;
  register unsigned long r21 asm ("r21") = arg6;
  /* FIXME: HACK save/load r19 around syscall */
  asm volatile (SAVE_ASM_PIC
		"ble  0x100(%%sr2, %%r0)\n"
		"copy %1, %%r20\n"
		LOAD_ASM_PIC
		: "=r" (res)
		: "r" (name) PIC_REG_USE,
		  "r" (r26), "r" (r25), "r" (r24), "r" (r23), "r" (r22),
		  "r" (r21)
		: "memory", CALL_CLOB_REGS);
  return res;
}

#endif	/* __ASSEMBLER__ */

/* Pointer mangling is not yet supported for HPPA.  */
#define PTR_MANGLE(var) (void) (var)
#define PTR_DEMANGLE(var) (void) (var)

#define SINGLE_THREAD_BY_GLOBAL	1

#endif /* _LINUX_HPPA_SYSDEP_H */
