/* Non-Temporal large memmove implementation.
   Copyright (C) 2025 Free Software Foundation, Inc.
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

#ifdef MEMMOVE_LARGE_IMPL
# error "Multiple large memmove impls included!"
#endif
#define MEMMOVE_LARGE_IMPL	1

/* Copies large regions by with a 4x unrolled loop of non-temporal
   stores.  */

#if (defined USE_MULTIARCH || VEC_SIZE == 16) && IS_IN (libc)
L(large_memcpy_check):
	cmp	__x86_shared_non_temporal_threshold(%rip), %RDX_LP
	jb	L(more_8x_vec_check)
L(large_memcpy):
	/* To reach this point it is impossible for dst > src and
	   overlap. Remaining to check is src > dst and overlap. rcx
	   already contains dst - src. Negate rcx to get src - dst. If
	   length > rcx then there is overlap and forward copy is best.  */
	negq	%rcx
	cmpq	%rcx, %rdx
	ja	L(more_8x_vec_forward)

	/* We are doing non-temporal copy and no overlap. Choose forward
	   or backward copy based on avoiding 4k aliasing. ecx already
	   contains src - dst. We check if:
	   (src % 4096) - (dst % 4096) > (4096 - 512)
	   If true then we risk aliasing.  */
	andl	$(PAGE_SIZE - 1), %ecx
	cmpl	$(PAGE_SIZE - 512), %ecx
	ja	L(large_backward)

	subq	%rdi, %rsi

	/* Store the first VEC.  */
	VMOVU	%VMM(0), (%rdi)

	/* Store end of buffer minus tail in rdx.  */
	leaq	(VEC_SIZE * -4)(%rdi, %rdx), %rdx

	/* Align DST.  */
	orq	$(VEC_SIZE - 1), %rdi
	incq	%rdi
	leaq	(%rdi, %rsi), %rcx
	/* Dont use multi-byte nop to align.  */
	.p2align 4,, 11
L(loop_4x_nt_forward):
	PREFETCH_ONE_SET (1, (%rcx), VEC_SIZE * 8)
	/* Copy 4 * VEC a time forward.  */
	VMOVU	(VEC_SIZE * 0)(%rcx), %VMM(1)
	VMOVU	(VEC_SIZE * 1)(%rcx), %VMM(2)
	VMOVU	(VEC_SIZE * 2)(%rcx), %VMM(3)
	VMOVU	(VEC_SIZE * 3)(%rcx), %VMM(4)
	subq	$-(VEC_SIZE * 4), %rcx
	VMOVNT	%VMM(1), (VEC_SIZE * 0)(%rdi)
	VMOVNT	%VMM(2), (VEC_SIZE * 1)(%rdi)
	VMOVNT	%VMM(3), (VEC_SIZE * 2)(%rdi)
	VMOVNT	%VMM(4), (VEC_SIZE * 3)(%rdi)
	subq	$-(VEC_SIZE * 4), %rdi
	cmpq	%rdi, %rdx
	ja	L(loop_4x_nt_forward)
	sfence

	VMOVU	(VEC_SIZE * 0)(%rsi, %rdx), %VMM(1)
	VMOVU	(VEC_SIZE * 1)(%rsi, %rdx), %VMM(2)
	VMOVU	(VEC_SIZE * 2)(%rsi, %rdx), %VMM(3)
	VMOVU	(VEC_SIZE * 3)(%rsi, %rdx), %VMM(4)
	VMOVU	%VMM(1), (VEC_SIZE * 0)(%rdx)
	VMOVU	%VMM(2), (VEC_SIZE * 1)(%rdx)
	VMOVU	%VMM(3), (VEC_SIZE * 2)(%rdx)
	VMOVU	%VMM(4), (VEC_SIZE * 3)(%rdx)
	VZEROUPPER_RETURN

	.p2align 4,, 10
L(large_backward):
	leaq	(VEC_SIZE * -4 - 1)(%rdi, %rdx), %rcx
	VMOVU	(VEC_SIZE * -1)(%rsi, %rdx), %VMM(5)
	VMOVU	%VMM(5), (VEC_SIZE * -1)(%rdi, %rdx)
	andq	$-(VEC_SIZE), %rcx
	subq	%rdi, %rsi
	leaq	(%rsi, %rcx), %rdx
	/* Don't use multi-byte nop to align.  */
	.p2align 4,, 11
L(loop_4x_nt_backward):
	PREFETCH_ONE_SET (-1, (%rdx), -VEC_SIZE * 8)
	VMOVU	(VEC_SIZE * 3)(%rdx), %VMM(1)
	VMOVU	(VEC_SIZE * 2)(%rdx), %VMM(2)
	VMOVU	(VEC_SIZE * 1)(%rdx), %VMM(3)
	VMOVU	(VEC_SIZE * 0)(%rdx), %VMM(4)
	addq	$(VEC_SIZE * -4), %rdx
	VMOVNT	%VMM(1), (VEC_SIZE * 3)(%rcx)
	VMOVNT	%VMM(2), (VEC_SIZE * 2)(%rcx)
	VMOVNT	%VMM(3), (VEC_SIZE * 1)(%rcx)
	VMOVNT	%VMM(4), (VEC_SIZE * 0)(%rcx)
	addq	$(VEC_SIZE * -4), %rcx
	cmpq	%rcx, %rdi
	jb	L(loop_4x_nt_backward)

	sfence
	VMOVU	(VEC_SIZE * 3)(%rsi, %rdi), %VMM(4)
	VMOVU	(VEC_SIZE * 2)(%rsi, %rdi), %VMM(3)
	VMOVU	(VEC_SIZE * 1)(%rsi, %rdi), %VMM(2)
	/* We already loaded VMM(0).  */
	VMOVU	%VMM(4), (VEC_SIZE * 3)(%rdi)
	VMOVU	%VMM(3), (VEC_SIZE * 2)(%rdi)
	VMOVU	%VMM(2), (VEC_SIZE * 1)(%rdi)
	VMOVU	%VMM(0), (VEC_SIZE * 0)(%rdi)
	VZEROUPPER_RETURN
#endif
