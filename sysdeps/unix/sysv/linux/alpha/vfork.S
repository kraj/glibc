/* Copyright (C) 2004-2020 Free Software Foundation, Inc.
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
#include <tcb-offsets.h>

#ifdef PIC
# define SYSCALL_ERROR_LABEL	__syscall_error !samegp
#else
# define SYSCALL_ERROR_LABEL	$syscall_error
#endif

ENTRY(__libc_vfork)
#ifdef PROF
	.set noat
	ldgp	gp,0(pv)
	lda	AT, _mcount;
	jsr	AT, (AT), _mcount;
	.set at
#else
	.prologue 0
#endif

	/* Load the thread pointer value in A1 across the vfork.  */
	rduniq
	mov	v0, a1

	lda	v0, __NR_vfork
	call_pal PAL_callsys

	/* Normal error check and return.  */
	bne	a3, SYSCALL_ERROR_LABEL
	ret

#ifndef PIC
$syscall_error:
	jmp	zero, __syscall_error
#endif
END (__libc_vfork)

#if IS_IN (libc)
weak_alias (__libc_vfork, vfork)
strong_alias (__libc_vfork, __vfork)
libc_hidden_def (__vfork)
#endif
