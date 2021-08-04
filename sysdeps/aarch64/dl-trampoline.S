/* Copyright (C) 2005-2021 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <libc-symbols.h>

#include "dl-link.h"

#define ip0 x16
#define ip0l PTR_REG (16)
#define ip1 x17
#define lr  x30

/* RELA relocatons are 3 pointers */
#define RELA_SIZE (PTR_SIZE * 3)

	.text
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, #function
	cfi_startproc
	.align 2
_dl_runtime_resolve:
	BTI_C
	/* AArch64 we get called with:
	   ip0		&PLTGOT[2]
	   ip1		temp(dl resolver entry point)
	   [sp, #8]	lr
	   [sp, #0]	&PLTGOT[n]
	 */

	cfi_rel_offset (lr, 8)

	/* Note: Saving x9 is not required by the ABI but the assember requires
	   the immediate values of operand 3 to be a multiple of 16 */
	stp	x8, x9, [sp, #-(80+8*16)]!
	cfi_adjust_cfa_offset (80+8*16)
	cfi_rel_offset (x8, 0)
	cfi_rel_offset (x9, 8)

	stp	x6, x7, [sp,  #16]
	cfi_rel_offset (x6, 16)
	cfi_rel_offset (x7, 24)

	stp	x4, x5, [sp,  #32]
	cfi_rel_offset (x4, 32)
	cfi_rel_offset (x5, 40)

	stp	x2, x3, [sp,  #48]
	cfi_rel_offset (x2, 48)
	cfi_rel_offset (x3, 56)

	stp	x0, x1, [sp,  #64]
	cfi_rel_offset (x0, 64)
	cfi_rel_offset (x1, 72)

	stp	q0, q1, [sp, #(80+0*16)]
	cfi_rel_offset (q0, 80+0*16)
	cfi_rel_offset (q1, 80+1*16)

	stp	q2, q3, [sp, #(80+2*16)]
	cfi_rel_offset (q0, 80+2*16)
	cfi_rel_offset (q1, 80+3*16)

	stp	q4, q5, [sp, #(80+4*16)]
	cfi_rel_offset (q0, 80+4*16)
	cfi_rel_offset (q1, 80+5*16)

	stp	q6, q7, [sp, #(80+6*16)]
	cfi_rel_offset (q0, 80+6*16)
	cfi_rel_offset (q1, 80+7*16)

	/* Get pointer to linker struct.  */
	ldr	PTR_REG (0), [ip0, #-PTR_SIZE]

	/* Prepare to call _dl_fixup().  */
	ldr	x1, [sp, 80+8*16]	/* Recover &PLTGOT[n] */

	sub     x1, x1, ip0
	add     x1, x1, x1, lsl #1
	lsl     x1, x1, #3
	sub     x1, x1, #(RELA_SIZE<<3)
	lsr     x1, x1, #3

	/* Call fixup routine.  */
	bl	_dl_fixup

	/* Save the return.  */
	mov	ip0, x0

	/* Get arguments and return address back.  */
	ldp	q0, q1, [sp, #(80+0*16)]
	ldp	q2, q3, [sp, #(80+2*16)]
	ldp	q4, q5, [sp, #(80+4*16)]
	ldp	q6, q7, [sp, #(80+6*16)]
	ldp	x0, x1, [sp, #64]
	ldp	x2, x3, [sp, #48]
	ldp	x4, x5, [sp, #32]
	ldp	x6, x7, [sp, #16]
	ldp	x8, x9, [sp], #(80+8*16)
	cfi_adjust_cfa_offset (-(80+8*16))

	ldp	ip1, lr, [sp], #16
	cfi_adjust_cfa_offset (-16)

	/* Jump to the newly found address.  */
	br	ip0

	cfi_endproc
	.size _dl_runtime_resolve, .-_dl_runtime_resolve
#ifndef PROF
	.globl _dl_runtime_profile
	.type _dl_runtime_profile, #function
	cfi_startproc
	.align 2
_dl_runtime_profile:
# if HAVE_AARCH64_PAC_RET
	PACIASP
	cfi_window_save
# else
	BTI_C
# endif
	/* AArch64 we get called with:
	   ip0		&PLTGOT[2]
	   ip1		temp(dl resolver entry point)
	   [sp, #8]	lr
	   [sp, #0]	&PLTGOT[n]

	   Stack frame layout:
	   [sp,   #...] lr
	   [sp,   #...] &PLTGOT[n]
	   -----------------------
	   [sp,   #384] La_aarch64_regs::lr_xreg (x0-x8)
	   [sp,   #256] La_aarch64_regs::lr_vreg (q0-q7)
	   [sp,   #240] La_aarch64_regs::sp and La_aarch64_regs::lr
	   [sp,   #176] La_aarch64_retval::lrv_xreg (x0-x7)
	   [sp,   # 48] La_aarch64_retval::lrv_vreg (q0-q7)
	   [sp,   # 40] frame size return from pltenter
	   [sp,   # 32] dl_profile_call saved x1
	   [sp,   # 24] dl_profile_call saved x0
	   [sp,   # 16] t1
	   [sp,   #  0] x29, lr   <- x29
	 */

# define OFFSET_T1		16
# define OFFSET_SAVED_CALL_X0	OFFSET_T1 + 8
# define OFFSET_FS		OFFSET_SAVED_CALL_X0 + 16
# define OFFSET_RV		OFFSET_FS + 8
# define OFFSET_RG		OFFSET_RV + DL_SIZEOF_RV

# define SF_SIZE		OFFSET_RG + DL_SIZEOF_RG

# define OFFSET_PLTGOTN		SF_SIZE
# define OFFSET_LR		OFFSET_PLTGOTN + 8

	/* Save arguments.  */
	sub	sp, sp, #SF_SIZE
	cfi_adjust_cfa_offset (SF_SIZE)
	stp	x29, x30, [SP, #0]
	mov	x29, sp
	cfi_def_cfa_register (x29)
	cfi_rel_offset (x29, 0)
	cfi_rel_offset (lr, 8)

	stp	x0, x1, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*0]
	cfi_rel_offset (x0, OFFSET_RG + DL_OFFSET_RG_X0 + 16*0 + 0)
	cfi_rel_offset (x1, OFFSET_RG + DL_OFFSET_RG_X0 + 16*0 + 8)
	stp	x2, x3, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*1]
	cfi_rel_offset (x2, OFFSET_RG + DL_OFFSET_RG_X0 + 16*1 + 0)
	cfi_rel_offset (x3, OFFSET_RG + DL_OFFSET_RG_X0 + 16*1 + 8)
	stp	x4, x5, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*2]
	cfi_rel_offset (x4, OFFSET_RG + DL_OFFSET_RG_X0 + 16*2 + 0)
	cfi_rel_offset (x5, OFFSET_RG + DL_OFFSET_RG_X0 + 16*2 + 8)
	stp	x6, x7, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*3]
	cfi_rel_offset (x6, OFFSET_RG + DL_OFFSET_RG_X0 + 16*3 + 0)
	cfi_rel_offset (x7, OFFSET_RG + DL_OFFSET_RG_X0 + 16*3 + 8)
	str	x8, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*4 + 0]
	cfi_rel_offset (x8, OFFSET_RG + DL_OFFSET_RG_X0 + 16*4 + 0)
	/* Note 8 bytes of padding is in the stack frame for alignment */

	stp	q0, q1, [X29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*0]
	cfi_rel_offset (q0, OFFSET_RG + DL_OFFSET_RG_V0 + 32*0)
	cfi_rel_offset (q1, OFFSET_RG + DL_OFFSET_RG_V0 + 32*0 + 16)
	stp	q2, q3, [X29, #OFFSET_RG+ DL_OFFSET_RG_V0 + 32*1]
	cfi_rel_offset (q2, OFFSET_RG + DL_OFFSET_RG_V0 + 32*1 + 0)
	cfi_rel_offset (q3, OFFSET_RG + DL_OFFSET_RG_V0 + 32*1 + 16)
	stp	q4, q5, [X29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*2]
	cfi_rel_offset (q4, OFFSET_RG + DL_OFFSET_RG_V0 + 32*2 + 0)
	cfi_rel_offset (q5, OFFSET_RG + DL_OFFSET_RG_V0 + 32*2 + 16)
	stp	q6, q7, [X29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*3]
	cfi_rel_offset (q6, OFFSET_RG + DL_OFFSET_RG_V0 + 32*3 + 0)
	cfi_rel_offset (q7, OFFSET_RG + DL_OFFSET_RG_V0 + 32*3 + 16)

	strb	wzr, [x29, #OFFSET_RG + DL_OFFSET_RG_SVE]
	strb	wzr, [x29, #OFFSET_RV + DL_OFFSET_RV_SVE]

	add     x0, x29, #SF_SIZE + 16
	ldr	x1, [x29, #OFFSET_LR]
	stp	x0, x1, [x29, #OFFSET_RG + DL_OFFSET_RG_SP]

	/* Get pointer to linker struct.  */
	ldr	PTR_REG (0), [ip0, #-PTR_SIZE]

	/* Prepare to call _dl_profile_fixup().  */
	ldr	x1, [x29, OFFSET_PLTGOTN]	/* Recover &PLTGOT[n] */

	sub     x1, x1, ip0
	add     x1, x1, x1, lsl #1
	lsl     x1, x1, #3
	sub     x1, x1, #(RELA_SIZE<<3)
	lsr     x1, x1, #3

	stp	x0, x1, [x29, #OFFSET_SAVED_CALL_X0]

	/* Set up extra args for _dl_profile_fixup */
	ldr	x2, [x29, #OFFSET_LR]		/* load saved LR */
	add	x3, x29, #OFFSET_RG		/* address of La_aarch64_reg */
	add	x4, x29, #OFFSET_FS		/* address of framesize */
	bl	_dl_profile_fixup

	ldr	ip0l, [x29, #OFFSET_FS]		/* framesize == 0 */
	cmp	ip0l, #0
	bge	1f
	cfi_remember_state

	/* Save the return.  */
	mov	ip0, x0

	/* Get arguments and return address back.  */
	ldp	x0, x1, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*0]
	ldp	x2, x3, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*1]
	ldp	x4, x5, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*2]
	ldp	x6, x7, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*3]
	ldr	x8,     [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*4]
	ldp	q0, q1, [x29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*0]
	ldp	q2, q3, [x29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*1]
	ldp	q4, q5, [x29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*2]
	ldp	q6, q7, [x29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*3]

	cfi_def_cfa_register (sp)
	ldp	x29, x30, [x29, #0]
	cfi_restore(x29)
	cfi_restore(x30)

# if HAVE_AARCH64_PAC_RET
	add	sp, sp, SF_SIZE
	cfi_adjust_cfa_offset (-SF_SIZE)
	AUTIASP
	cfi_window_save
	add	sp, sp, 16
	cfi_adjust_cfa_offset (-16)
# else
	add	sp, sp, SF_SIZE + 16
	cfi_adjust_cfa_offset (- SF_SIZE - 16)
# endif

	/* Jump to the newly found address.  */
	br	ip0

	cfi_restore_state
1:
	/* The new frame size is in ip0.  */

	sub	PTR_REG (1), PTR_REG (29), ip0l
	and	sp, x1, #0xfffffffffffffff0

	str	x0, [x29, #OFFSET_T1]

	mov	x0, sp
	add	x1, x29, #SF_SIZE + 16
	mov	x2, ip0
	bl	memcpy

	ldr	ip0, [x29, #OFFSET_T1]

	/* Call the function.  */
	ldp	x0, x1, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*0]
	ldp	x2, x3, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*1]
	ldp	x4, x5, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*2]
	ldp	x6, x7, [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*3]
	ldr	x8,     [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*4]
	ldp	q0, q1, [x29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*0]
	ldp	q2, q3, [x29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*1]
	ldp	q4, q5, [x29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*2]
	ldp	q6, q7, [x29, #OFFSET_RG + DL_OFFSET_RG_V0 + 32*3]
	blr	ip0
	stp	x0, x1, [x29, #OFFSET_RV + DL_OFFSET_RV_X0 + 16*0]
	stp	x2, x3, [x29, #OFFSET_RV + DL_OFFSET_RV_X0 + 16*1]
	stp	x4, x5, [x29, #OFFSET_RV + DL_OFFSET_RV_X0 + 16*2]
	stp	x6, x7, [x29, #OFFSET_RV + DL_OFFSET_RV_X0 + 16*3]
	str	x8,     [x29, #OFFSET_RG + DL_OFFSET_RG_X0 + 16*4]
	stp	q0, q1, [x29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*0]
	stp	q2, q3, [x29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*1]
	stp	q4, q5, [x29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*2]
	stp	q6, q7, [x29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*3]

	/* Setup call to pltexit  */
	ldp	x0, x1, [x29, #OFFSET_SAVED_CALL_X0]
	add	x2, x29, #OFFSET_RG
	add	x3, x29, #OFFSET_RV
	bl	_dl_audit_pltexit

	ldp	x0, x1, [x29, #OFFSET_RV + DL_OFFSET_RV_X0 + 16*0]
	ldp	x2, x3, [x29, #OFFSET_RV + DL_OFFSET_RV_X0 + 16*1]
	ldp	x4, x5, [x29, #OFFSET_RV + DL_OFFSET_RV_X0 + 16*2]
	ldp	x6, x7, [x29, #OFFSET_RV + DL_OFFSET_RV_X0 + 16*3]
	ldr	x8,     [x29, #OFFSET_RV + DL_OFFSET_RV_X0 + 16*4]
	ldp	q0, q1, [x29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*0]
	ldp	q2, q3, [x29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*1]
	ldp	q4, q5, [x29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*2]
	ldp	q6, q7, [x29, #OFFSET_RV + DL_OFFSET_RV_V0 + 32*3]

	/* LR from within La_aarch64_reg */
	ldr	lr, [x29, #OFFSET_RG + DL_OFFSET_RG_LR]
	cfi_restore(lr)
# if HAVE_AARCH64_PAC_RET
	/* Note: LR restored from La_aarch64_reg has no PAC.  */
	cfi_window_save
# endif
	mov	sp, x29
	cfi_def_cfa_register (sp)
	ldr	x29, [x29, #0]
	cfi_restore(x29)
	add	sp, sp, SF_SIZE + 16
	cfi_adjust_cfa_offset (- SF_SIZE - 16)

	br	lr

	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
	.previous


#define HWCAP_SVE		22
#define ZCR_ELx_LEN_MASK        0x1ff

#if HAVE_AARCH64_SVE_ASM
	.arch armv8.2-a+sve
	.globl _dl_runtime_profile_sve
	.type _dl_runtime_profile_sve, #function
	cfi_startproc
	.align 2
_dl_runtime_profile_sve:
# if HAVE_AARCH64_PAC_RET
	PACIASP
	cfi_window_save
# else
	BTI_C
# endif
        /* AArch64 we get called with:
           ip0          &PLTGOT[2]
           ip1          temp(dl resolver entry point)
           [sp, #8]     lr
           [sp, #0]     &PLTGOT[n]

           Stack frame layout:
           [sp,   #...] lr
           [sp,   #...] &PLTGOT[n]
           -------------------------
	   TODO
           [sp,   #  0] x29, lr   <- x29
	 */

# define OFFSET_SVE_T1			16
# define OFFSET_SVE_SAVED_CALL_X0	OFFSET_SVE_T1 + 8
# define OFFSET_SVE_FS			OFFSET_SVE_SAVED_CALL_X0 + 16
# define OFFSET_SVE_RV			OFFSET_SVE_FS + 8
# define OFFSET_SVE_RG			OFFSET_SVE_RV + DL_SIZEOF_RV
/* Maximum supported z and pregisters size in bytes.  */
# define SIZEOF_SVE_Z_REG		512
# define SIZEOF_SVE_P_REG		SIZEOF_SVE_Z_REG / 8
/* z0-z7 for argument passing.  */
# define SIZEOF_SVE_RG_Z		8 * SIZEOF_SVE_Z_REG
/* p0-p3 for argument passing.  */
# define SIZEOF_SVE_RG_P		4 * SIZEOF_SVE_P_REG
/* z0-z7 for function return.  */
# define SIZEOF_SVE_RV_P		8 * SIZEOF_SVE_Z_REG
/* SVE registers contents for La_aarch64_regs.lr_vreg  */
# define OFFSET_SVE_RG_Z		OFFSET_SVE_RG + DL_SIZEOF_RG
/* SVE registers contents for La_aarch64.regs.lr_sve_pregs  */
# define OFFSET_SVE_RG_P		OFFSET_SVE_RG_Z + SIZEOF_SVE_RG_Z
/* SVE registers contents for La_aarch64_retval.lrv_vreg  */
# define OFFSET_SVE_RV_Z		OFFSET_SVE_RG_P + SIZEOF_SVE_RG_P

# define SF_SVE_SIZE			OFFSET_SVE_RV_Z + SIZEOF_SVE_RV_P

# define OFFSET_SVE_PLTGOTN		SF_SVE_SIZE
# define OFFSET_SVE_LR			OFFSET_SVE_PLTGOTN + 8

	.macro save_sve_z_reg zreg idx offset save_addr
	.ifc \zreg, z0
	.if \offset < 4096
	add	x0, x29, \offset
	.else
	mov	x0, \offset
	add	x0, x29, x0
	.endif
	.else
	add	x0, x0, SIZEOF_SVE_Z_REG
	.endif
	str	\zreg, [x0, #0]
	.if \save_addr == 1
	str	x0, [X29, #OFFSET_RG + DL_OFFSET_RG_V0 + 16*\idx]
	.else
	str	x0, [X29, #OFFSET_RV + DL_OFFSET_RV_V0 + 16*\idx]
	.endif
	.endm

	.macro save_sve_regs offset save_addr
	save_sve_z_reg z0 0 \offset \save_addr
	save_sve_z_reg z1 1 \offset \save_addr
	save_sve_z_reg z2 2 \offset \save_addr
	save_sve_z_reg z3 3 \offset \save_addr
	save_sve_z_reg z4 4 \offset \save_addr
	save_sve_z_reg z5 5 \offset \save_addr
	save_sve_z_reg z6 6 \offset \save_addr
	save_sve_z_reg z7 7 \offset \save_addr
	.if \save_addr  == 1
	add	x0, x0, SIZEOF_SVE_P_REG
	str	p0, [x0, #0]
	add	x0, x0, SIZEOF_SVE_P_REG
	str	p1, [x0, #0]
	add	x0, x0, SIZEOF_SVE_P_REG
	str	p2, [x0, #0]
	add	x0, x0, SIZEOF_SVE_P_REG
	str	p3, [x0, #0]
	.endif
	.endm

	.macro load_sve_regs offset
	.if \offset < 4096
	add	x12, x29, \offset
	.else
	mov	x12, \offset
	add	x12, x29, x12
	.endif
	ldr	z0, [x12, #0]
	add	x12, x12, SIZEOF_SVE_Z_REG
	ldr	z1, [x12, #0]
	add	x12, x12, SIZEOF_SVE_Z_REG
	ldr	z2, [x12, #0]
	add	x12, x12, SIZEOF_SVE_Z_REG
	ldr	z3, [x12, #0]
	add	x12, x12, SIZEOF_SVE_Z_REG
	ldr	z4, [x12, #0]
	add	x12, x12, SIZEOF_SVE_Z_REG
	ldr	z5, [x12, #0]
	add	x12, x12, SIZEOF_SVE_Z_REG
	ldr	z6, [x12, #0]
	add	x12, x12, SIZEOF_SVE_Z_REG
	ldr	z7, [x12, #0]
	add	x12, x12, SIZEOF_SVE_P_REG
	ldr	p0, [x12, #0]
	add	x12, x12, SIZEOF_SVE_P_REG
	ldr	p1, [x12, #0]
	add	x12, x12, SIZEOF_SVE_P_REG
	ldr	p2, [x12, #0]
	add	x12, x12, SIZEOF_SVE_P_REG
	ldr	p3, [x12, #0]
	.endm


	/* Save arguments.  */
	mov	x12, #SF_SVE_SIZE
	sub	sp, sp, x12
	cfi_adjust_cfa_offset (SF_SVE_SIZE)
	stp	x29, x30, [SP, #0]
	mov	x29, sp
	cfi_def_cfa_register (x29)
	cfi_rel_offset (x29, 0)
	cfi_rel_offset (lr, 8)

	stp	x0, x1, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*0]
	cfi_rel_offset (x0, OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*0 + 0)
	cfi_rel_offset (x1, OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*0 + 8)
	stp	x2, x3, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*1]
	cfi_rel_offset (x2, OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*1 + 0)
	cfi_rel_offset (x3, OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*1 + 8)
	stp	x4, x5, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*2]
	cfi_rel_offset (x4, OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*2 + 0)
	cfi_rel_offset (x5, OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*2 + 8)
	stp	x6, x7, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*3]
	cfi_rel_offset (x6, OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*3 + 0)
	cfi_rel_offset (x7, OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*3 + 8)
	str	x8, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*4 + 0]
	cfi_rel_offset (x8, OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*4 + 0)
	/* Note 8 bytes of padding is in the stack frame for alignment */
	save_sve_regs OFFSET_SVE_RG_Z 1

	/* Store the vector length on lr_sve  */
	mov	x0, #0
	addvl	x0, x0, #1
	strb	w0, [x29, #OFFSET_RG + DL_OFFSET_RG_SVE]
	strb	w0, [x29, #OFFSET_RV + DL_OFFSET_RV_SVE]

	mov	x0, #SF_SVE_SIZE + 16
	add     x0, x29, x0
	ldr	x1, [x29, #OFFSET_SVE_LR]
	stp	x0, x1, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_SP]

	/* Get pointer to linker struct.  */
	ldr	PTR_REG (0), [ip0, #-PTR_SIZE]

	/* Prepare to call _dl_profile_fixup().  */
	ldr	x1, [x29, OFFSET_SVE_PLTGOTN]	/* Recover &PLTGOT[n] */

	sub     x1, x1, ip0
	add     x1, x1, x1, lsl #1
	lsl     x1, x1, #3
	sub     x1, x1, #(RELA_SIZE<<3)
	lsr     x1, x1, #3

	stp	x0, x1, [x29, #OFFSET_SVE_SAVED_CALL_X0]

	/* Set up extra args for _dl_profile_fixup */
	ldr	x2, [x29, #OFFSET_SVE_LR]	/* load saved LR */
	add	x3, x29, #OFFSET_SVE_RG		/* address of La_aarch64_reg */
	add	x4, x29, #OFFSET_SVE_FS		/* address of framesize */
	bl	_dl_profile_fixup

	ldr	ip0l, [x29, #OFFSET_SVE_FS]	/* framesize == 0 */
	cmp	ip0l, #0
	bge	1f
	cfi_remember_state

	/* Save the return.  */
	mov	ip0, x0

	/* Get arguments and return address back.  */
	ldp	x0, x1, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*0]
	ldp	x2, x3, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*1]
	ldp	x4, x5, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*2]
	ldp	x6, x7, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*3]
	ldr	x8,     [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*4]
	load_sve_regs OFFSET_SVE_RG_Z

	cfi_def_cfa_register (sp)
	ldp	x29, x30, [x29, #0]
	cfi_restore (x29)
	cfi_restore (x30)

# if HAVE_AARCH64_PAC_RET
	mov	x12, SF_SVE_SIZE
	add	sp, sp, x12
	cfi_adjust_cfa_offset (-SF_SVE_SIZE)
	AUTIASP
	cfi_window_save
	add	sp, sp, 16
	cfi_adjust_cfa_offset (-16)
# else
	mov	x12, SF_SVE_SIZE + 16
	add	sp, sp, x12
	cfi_adjust_cfa_offset (- SF_SVE_SIZE - 16)
# endif

	/* Jump to the newly found address.  */
	br	ip0

	cfi_restore_state
1:
	/* The new frame size is in ip0.  */

	sub	PTR_REG (1), PTR_REG (29), ip0l
	and	sp, x1, #0xfffffffffffffff0

	str	x0, [x29, #OFFSET_SVE_T1]

	mov	x0, sp
	mov	x1, #SF_SVE_SIZE + 16
	add	x1, x29, x1
	mov	x2, ip0
	bl	memcpy

	ldr	ip0, [x29, #OFFSET_SVE_T1]

	/* Call the function.  */
	ldp	x0, x1, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*0]
	ldp	x2, x3, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*1]
	ldp	x4, x5, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*2]
	ldp	x6, x7, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*3]
	ldr	x8,     [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*4]
	load_sve_regs OFFSET_SVE_RG_Z
	blr	ip0
	stp	x0, x1, [x29, #OFFSET_SVE_RV + DL_OFFSET_RV_X0 + 16*0]
	stp	x2, x3, [x29, #OFFSET_SVE_RV + DL_OFFSET_RV_X0 + 16*1]
	stp	x4, x5, [x29, #OFFSET_SVE_RV + DL_OFFSET_RV_X0 + 16*2]
	stp	x6, x7, [x29, #OFFSET_SVE_RV + DL_OFFSET_RV_X0 + 16*3]
	str	x8,     [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_X0 + 16*4]
	save_sve_regs OFFSET_SVE_RV_Z 0

	/* Setup call to pltexit  */
	ldp	x0, x1, [x29, #OFFSET_SVE_SAVED_CALL_X0]
	add	x2, x29, #OFFSET_SVE_RG
	add	x3, x29, #OFFSET_SVE_RV
	bl	_dl_audit_pltexit

	ldp	x0, x1, [x29, #OFFSET_SVE_RV + DL_OFFSET_RV_X0 + 16*0]
	ldp	x2, x3, [x29, #OFFSET_SVE_RV + DL_OFFSET_RV_X0 + 16*1]
	ldp	x4, x5, [x29, #OFFSET_SVE_RV + DL_OFFSET_RV_X0 + 16*2]
	ldp	x6, x7, [x29, #OFFSET_SVE_RV + DL_OFFSET_RV_X0 + 16*3]
	ldr	x8,     [x29, #OFFSET_SVE_RV + DL_OFFSET_RV_X0 + 16*4]
	load_sve_regs OFFSET_SVE_RV_Z

	/* LR from within La_aarch64_reg */
	ldr	lr, [x29, #OFFSET_SVE_RG + DL_OFFSET_RG_LR]
	cfi_restore(lr)
# if HAVE_AARCH64_PAC_RET
	/* Note: LR restored from La_aarch64_reg has no PAC.  */
	cfi_window_save
# endif
	mov	sp, x29
	cfi_def_cfa_register (sp)
	ldr	x29, [x29, #0]
	cfi_restore(x29)
	mov	x12, SF_SVE_SIZE + 16
	add	sp, sp, x12
	cfi_adjust_cfa_offset (- SF_SVE_SIZE - 16)

	br	lr

	cfi_endproc
	.size _dl_runtime_profile_sve, .-_dl_runtime_profile_sve
	.previous
#endif /* HAVE_AARCH64_SVE_ASM  */
