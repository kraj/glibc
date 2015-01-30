/* Optimized memcpy implementation for PowerPC64/POWER8.
   Copyright (C) 2017 Free Software Foundation, Inc.
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


/* __ptr_t [r3] memcpy (__ptr_t dst [r3], __ptr_t src [r4], size_t len [r5]);
   Returns 'dst'.  */

#ifndef MEMCPY
# define MEMCPY memcpy
#endif

	.machine power8
ENTRY_TOCLESS (MEMCPY, 5)
	CALL_MCOUNT 3

	cmpldi	7,5,15
	bgt	7,L(copy_gt_32)
	andi.	9,5,0x1
	mr	9,3
	beq	0,1f
	lbz	10,0(4)
	addi	9,3,1
	addi	4,4,1
	stb	10,0(3)
1:
	andi.	10,5,0x2
	beq	0,2f
	lhz	10,0(4)
	addi	9,9,2
	addi	4,4,2
	sth	10,-2(9)
2:
	andi.	10,5,0x4
	beq	0,3f
	lwz	10,0(4)
	addi	9,9,4
	addi	4,4,4
	stw	10,-4(9)
3:
	andi.	10,5,0x8
	beqlr	0
	ld	10,0(4)
	std	10,0(9)
	blr

	.align 4
L(copy_gt_32):
	cmpldi	7,5,32
	ble	7,L(copy_gt_16_le_32)
	cmpldi	7,5,64
	ble	7,L(copy_gt_32_le_64)
	cmpldi	7,5,127
	mr	9,3
	ble	7,L(copy_gt_64_le_128)
	addi	8,5,-128
	li	11,16
	rldicr	8,8,0,56
	li	6,32
	srdi	10,8,7
	addi	0,8,128
	addi	10,10,1
	li	7,48
	mtctr	10

	.align 4
L(copy_128):
	lxvd2x	10,0,4
	lxvd2x	11,4,11
	addi	8,4,64
	addi	10,9,64
	lxvd2x	12,4,6
	lxvd2x	0,4,7
	addi	4,4,128
	stxvd2x	10,0,9
	stxvd2x	11,9,11
	stxvd2x	12,9,6
	stxvd2x	0,9,7
	addi	9,9,128
	lxvd2x	10,0,8
	lxvd2x	11,8,11
	lxvd2x	12,8,6
	lxvd2x	0,8,7
	stxvd2x	10,0,10
	stxvd2x	11,10,11
	stxvd2x	12,10,6
	stxvd2x	0,10,7
	bdnz	L(copy_128)
	add	9,3,0
	rldicl	5,5,0,57
L(copy_gt_64_le_128):
	cmpldi	7,5,63
	ble	7,L(copy_tail_le_64)
	li	7,16
	li	8,32
	lxvd2x	10,0,4
	li	10,48
	addi	5,5,-64
	lxvd2x	11,4,7
	lxvd2x	12,4,8
	lxvd2x	0,4,10
	addi	4,4,64
	stxvd2x	10,0,9
	stxvd2x	11,9,7
	stxvd2x	12,9,8
	stxvd2x	0,9,10
	addi	9,9,64
L(copy_tail_le_64):
	cmpldi	7,5,32
	bgt	7,L(copy_tail_gt_32_le_64)
	cmpdi	7,5,0
	beqlr	7
	addi	5,5,-32
	li	10,16
	add	7,4,5
	add	8,9,5
	lxvd2x	12,4,5
	lxvd2x	0,7,10
	stxvd2x	12,9,5
	stxvd2x	0,8,10
	blr

	.align 4
L(copy_gt_16_le_32):
	lxvd2x	0,0,4
	addi	5,5,-16
	lxvd2x	12,4,5
	stxvd2x 0,0,3
	stxvd2x 12,3,5
	blr

	.align 4
L(copy_gt_32_le_64):
	li	9,16
	lxvd2x	12,0,4
	addi	5,5,-32
	add	8,4,5
	add	10,3,5
	lxvd2x	0,4,9
	stxvd2x	12,0,3
	stxvd2x	0,3,9
	lxvd2x	12,4,5
	lxvd2x	0,8,9
	stxvd2x	12,3,5
	stxvd2x	0,10,9
	blr

	.align 4
L(copy_tail_gt_32_le_64):
	li	10,16
	lxvd2x	12,0,4
	addi	5,5,-32
	add	7,4,5
	add	8,9,5
	lxvd2x	0,4,10
	stxvd2x	12,0,9
	stxvd2x	0,9,10
	lxvd2x	12,4,5
	lxvd2x	0,7,10
	stxvd2x	12,9,5
	stxvd2x	0,8,10
	blr

END_GEN_TB (MEMCPY,TB_TOCLESS)
libc_hidden_builtin_def (memcpy)
