/* Copyright (C) 1996-2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* Please consult the file sysdeps/unix/sysv/linux/m68k/sysdep.h for
   more information about the value -4095 used below.*/

	.text
ENTRY (syscall)
	move.l	4(%sp), %d0	/* Load syscall number.  */
	move.l	%d5, -(%sp)
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (%d5, 0)
	move.l	28 (%sp), %d5
	move.l	%d4, -(%sp)
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (%d4, 0)
	move.l	28 (%sp), %d4
	move.l	%d3, %a1
	cfi_register (%d3, %a1)
	move.l	24(%sp), %d3
	move.l %d2, %a0
	cfi_register (%d2, %a0)
	move.l	20(%sp), %d2
	move.l	16(%sp), %d1
	move.l	%a0, -(%sp)
	cfi_adjust_cfa_offset (4)
	move.l	40(%sp), %a0
	trap &0			/* Do the system call.  */
	move.l	(%sp)+, %a0
	cfi_adjust_cfa_offset (-4)
	move.l	%a0, %d2
	cfi_restore (%d2)
	move.l %a1, %d3
	cfi_restore (%d3)
	move.l (%sp)+, %d4
	cfi_adjust_cfa_offset (-4)
	cfi_restore (%d4)
	move.l (%sp)+, %d5
	cfi_adjust_cfa_offset (-4)
	cfi_restore (%d5)
	cmp.l &-4095, %d0	/* Check %d0 for error.  */
	jcc SYSCALL_ERROR_LABEL	/* Jump to error handler if negative.  */
	rts			/* Return to caller.  */

	SYSCALL_ERROR_HANDLER
END (syscall)
