/* Assembler macros for HP/PA.
   Copyright (C) 1999-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@cygnus.com>, August 1999.

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

#include <sysdeps/generic/sysdep.h>

#undef ASM_LINE_SEP
#define ASM_LINE_SEP !

#ifdef	__ASSEMBLER__

/* Syntactic details of assembler.  */

#define ALIGNARG(log2) log2


/* Define an entry point visible from C.

   There is currently a bug in gdb which prevents us from specifying
   incomplete stabs information.  Fake some entries here which specify
   the current source file.  */
#define	ENTRY(name)							\
	.text						ASM_LINE_SEP	\
	.align ALIGNARG(4)				ASM_LINE_SEP	\
	.export C_SYMBOL_NAME(name)			ASM_LINE_SEP	\
	.type	C_SYMBOL_NAME(name),@function		ASM_LINE_SEP	\
	cfi_startproc					ASM_LINE_SEP	\
	C_LABEL(name)					ASM_LINE_SEP	\
	.PROC						ASM_LINE_SEP	\
	.CALLINFO FRAME=64,CALLS,SAVE_RP,ENTRY_GR=3	ASM_LINE_SEP	\
	.ENTRY						ASM_LINE_SEP	\
	/* SAVE_RP says we do */			ASM_LINE_SEP	\
	stw %rp, -20(%sr0,%sp)				ASM_LINE_SEP	\
	.cfi_offset 2, -20				ASM_LINE_SEP	\
	/*FIXME: Call mcount? (carefull with stack!) */

/* Some syscall wrappers do not call other functions, and
   hence are classified as leaf, so add NO_CALLS for gdb */
#define	ENTRY_LEAF(name)						\
	.text						ASM_LINE_SEP	\
	.align ALIGNARG(4)				ASM_LINE_SEP	\
	.export C_SYMBOL_NAME(name)			ASM_LINE_SEP	\
	.type	C_SYMBOL_NAME(name),@function		ASM_LINE_SEP	\
	cfi_startproc					ASM_LINE_SEP	\
	C_LABEL(name)					ASM_LINE_SEP	\
	.PROC						ASM_LINE_SEP	\
	.CALLINFO FRAME=64,NO_CALLS,SAVE_RP,ENTRY_GR=3	ASM_LINE_SEP	\
	.ENTRY						ASM_LINE_SEP	\
	/* SAVE_RP says we do */			ASM_LINE_SEP	\
	stw %rp, -20(%sr0,%sp)				ASM_LINE_SEP	\
	.cfi_offset 2, -20				ASM_LINE_SEP	\
	/*FIXME: Call mcount? (carefull with stack!) */

#undef END
#define END(name)							\
	.EXIT						ASM_LINE_SEP	\
	.PROCEND					ASM_LINE_SEP	\
	cfi_endproc					ASM_LINE_SEP	\
.size	C_SYMBOL_NAME(name), .-C_SYMBOL_NAME(name)	ASM_LINE_SEP

/* If compiled for profiling, call `mcount' at the start
   of each function. No, don't bother.  gcc will put the
   call in for us.  */
#define CALL_MCOUNT		/* Do nothing.  */

#undef JUMPTARGET
#define JUMPTARGET(name)	name
#define SYSCALL_PIC_SETUP	/* Nothing.  */

/* Local label name for asm code. */
#ifndef L
#define L(name)		name
#endif

#endif	/* __ASSEMBLER__ */
