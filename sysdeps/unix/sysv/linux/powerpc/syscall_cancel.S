/* Cancellable syscall wrapper.  Linux/powerpc version.
   Copyright (C) 2023 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <descr-const.h>

/* long int [r3] __syscall_cancel_arch (int *cancelhandling [r3],
					long int nr   [r4],
					long int arg1 [r5],
					long int arg2 [r6],
					long int arg3 [r7],
					long int arg4 [r8],
					long int arg5 [r9],
					long int arg6 [r10])  */

ENTRY (__syscall_cancel_arch)

	.globl __syscall_cancel_arch_start
__syscall_cancel_arch_start:

	/* if (*cancelhandling & CANCELED_BITMASK)
	     __syscall_do_cancel()  */
	lwz     r0,0(r3)
	andi.   r0,r0,TCB_CANCELED_BITMASK
	bne-    1f

	/* Issue a 6 argument syscall, the nr [r4] being the syscall
	   number.  */
	mr      r0,r4
	mr      r3,r5
	mr      r4,r6
	mr      r5,r7
	mr      r6,r8
	mr      r7,r9
	mr      r8,r10
	sc

	.globl __syscall_cancel_arch_end
__syscall_cancel_arch_end:

	bnslr+
	neg	r3,r3
	blr

	/* Although the __syscall_do_cancel do not return, we need to stack
	   being set correctly for unwind.  */
1:
	TAIL_CALL_NO_RETURN (__syscall_do_cancel)

END (__syscall_cancel_arch)
