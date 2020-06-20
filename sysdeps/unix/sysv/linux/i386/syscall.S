/* Copyright (C) 1995-2020 Free Software Foundation, Inc.
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

#include <sysdep.h>

/* Please consult the file sysdeps/unix/sysv/linux/i386/sysdep.h for
   more information about the value -4095 used below.*/

	.text
ENTRY (syscall)
	/* Save register contents.  */
	pushl %ebp
 	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebp, 0)
	pushl %edi
 	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (edi, 0)
 	pushl %esi
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (esi, 0)
 	pushl %ebx
 	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)

	/* Load arguments.  */
	movl 44(%esp), %ebp
 	movl 40(%esp), %edi
 	movl 36(%esp), %esi
 	movl 32(%esp), %edx
	movl 28(%esp), %ecx
	movl 24(%esp), %ebx
	movl 20(%esp), %eax

	ENTER_KERNEL		/* Do the system call.  */

	/* Restore register contents.  */
	popl %ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
	popl %esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	popl %edi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (edi)
	popl %ebp
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebp)

	cmpl $-4095, %eax	/* Check %eax for error.  */
	jae __syscall_error	/* Jump to error handler if error.  */
	ret			/* Return to caller.  */
END (syscall)
