/* Cancellable syscall wrapper.  Linux/IA64 version.
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
#undef ret

/* long int __syscall_cancel_arch (int *cancelhandling, long int nr,
				   long int arg1, long int arg2, long int arg3,
				   long int arg4, long int arg5, long int arg6)
*/

ENTRY (__syscall_cancel_arch)
	.prologue ASM_UNW_PRLG_RP | ASM_UNW_PRLG_PFS, ASM_UNW_PRLG_GRSAVE (8)
	.mmi
	.save ar.pfs, r41
	alloc r41=ar.pfs,8,3,8,0
	mov r15=r33
	.save rp, r40
	mov r40=b0
	.body
	.mmi
	mov r43=r34
	mov r44=r35
	mov r45=r36
	;;
	.mmi
	mov r46=r37
	mov r47=r38
	mov r48=r39
	;;

	.global __syscall_cancel_arch_start
__syscall_cancel_arch_start:
	;;
	.mmi
	nop 0
	ld4.acq r14=[r32]
	nop 0
	;;
	.mib
	nop 0
	tbit.z p6, p7=r14, TCB_CANCELED_BIT
	.pred.safe_across_calls p1-p63
(p7)	br.call.dpnt.many b0 = __syscall_do_cancel#
	.pred.safe_across_calls p1-p5,p16-p63
	;;

	/* Due instruction bundle ia64 has the end marker before the syscall
           instruction.  Check IA64 ucontext_check_pc_boundary on how the PC
           is checked.  */
	.global __syscall_cancel_arch_end
__syscall_cancel_arch_end:
	break 0x100000
	;;
	.mmi
	cmp.ne p6, p7=-1, r10
	nop 0
	mov b0=r40
	;;
	.mib
(p7)	sub r8=r0, r8
	mov ar.pfs=r41
	br.ret.sptk.many b0

END (__syscall_cancel_arch)
