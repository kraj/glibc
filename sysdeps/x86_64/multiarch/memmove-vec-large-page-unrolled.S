/* Non-Temporal page unrolled large memmove implementation.
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

/* Copies large regions by copying multiple pages at once.  This is
	beneficial on some older Intel hardware (Broadwell, Skylake, and
	Icelake).
   1. If size < 16 * __x86_shared_non_temporal_threshold and
      source and destination do not page alias, copy from 2 pages
      at once using non-temporal stores. Page aliasing in this case is
      considered true if destination's page alignment - sources' page
      alignment is less than 8 * VEC_SIZE.
   2. If size >= 16 * __x86_shared_non_temporal_threshold or source
      and destination do page alias copy from 4 pages at once using
      non-temporal stores.  */

#ifndef LOG_PAGE_SIZE
# define LOG_PAGE_SIZE	12
#endif

#if PAGE_SIZE != (1 << LOG_PAGE_SIZE)
# error Invalid LOG_PAGE_SIZE
#endif

/* Byte per page for large_memcpy inner loop.  */
#if VEC_SIZE == 64
# define LARGE_LOAD_SIZE	(VEC_SIZE * 2)
#else
# define LARGE_LOAD_SIZE	(VEC_SIZE * 4)
#endif

/* Amount to shift __x86_shared_non_temporal_threshold by for
   bound for memcpy_large_4x. This is essentially use to to
   indicate that the copy is far beyond the scope of L3
   (assuming no user config x86_non_temporal_threshold) and to
   use a more aggressively unrolled loop.  NB: before
   increasing the value also update initialization of
   x86_non_temporal_threshold.  */
#ifndef LOG_4X_MEMCPY_THRESH
# define LOG_4X_MEMCPY_THRESH	4
#endif

#if LARGE_LOAD_SIZE == (VEC_SIZE * 2)
# define LOAD_ONE_SET(base, offset, vec0, vec1, ...)	\
	VMOVU	(offset)base, vec0;	\
	VMOVU	((offset) + VEC_SIZE)base, vec1;
# define STORE_ONE_SET(base, offset, vec0, vec1, ...)	\
	VMOVNT	vec0, (offset)base;	\
	VMOVNT	vec1, ((offset) + VEC_SIZE)base;
#elif LARGE_LOAD_SIZE == (VEC_SIZE * 4)
# define LOAD_ONE_SET(base, offset, vec0, vec1, vec2, vec3)	\
	VMOVU	(offset)base, vec0;	\
	VMOVU	((offset) + VEC_SIZE)base, vec1;	\
	VMOVU	((offset) + VEC_SIZE * 2)base, vec2;	\
	VMOVU	((offset) + VEC_SIZE * 3)base, vec3;
# define STORE_ONE_SET(base, offset, vec0, vec1, vec2, vec3)	\
	VMOVNT	vec0, (offset)base;	\
	VMOVNT	vec1, ((offset) + VEC_SIZE)base;	\
	VMOVNT	vec2, ((offset) + VEC_SIZE * 2)base;	\
	VMOVNT	vec3, ((offset) + VEC_SIZE * 3)base;
#else
# error Invalid LARGE_LOAD_SIZE
#endif

	.p2align 4,, 10
#if (defined USE_MULTIARCH || VEC_SIZE == 16) && IS_IN (libc)
L(large_memcpy_check):
	/* Entry from L(large_memcpy_2x) has a redundant load of
	   __x86_shared_non_temporal_threshold(%rip). L(large_memcpy_2x)
	   is only use for the non-erms memmove which is generally less
	   common.  */
L(large_memcpy):
	mov	__x86_shared_non_temporal_threshold(%rip), %R11_LP
	cmp	%R11_LP, %RDX_LP
	jb	L(more_8x_vec_check)
	/* To reach this point it is impossible for dst > src and
	   overlap. Remaining to check is src > dst and overlap. rcx
	   already contains dst - src. Negate rcx to get src - dst. If
	   length > rcx then there is overlap and forward copy is best.  */
	negq	%rcx
	cmpq	%rcx, %rdx
	ja	L(more_8x_vec_forward)

	/* Cache align destination. First store the first 64 bytes then
	   adjust alignments.  */

	/* First vec was also loaded into VEC(0).  */
# if VEC_SIZE < 64
	VMOVU	VEC_SIZE(%rsi), %VMM(1)
#  if VEC_SIZE < 32
	VMOVU	(VEC_SIZE * 2)(%rsi), %VMM(2)
	VMOVU	(VEC_SIZE * 3)(%rsi), %VMM(3)
#  endif
# endif
	VMOVU	%VMM(0), (%rdi)
# if VEC_SIZE < 64
	VMOVU	%VMM(1), VEC_SIZE(%rdi)
#  if VEC_SIZE < 32
	VMOVU	%VMM(2), (VEC_SIZE * 2)(%rdi)
	VMOVU	%VMM(3), (VEC_SIZE * 3)(%rdi)
#  endif
# endif

	/* Adjust source, destination, and size.  */
	movq	%rdi, %r8
	andq	$63, %r8
	/* Get the negative of offset for alignment.  */
	subq	$64, %r8
	/* Adjust source.  */
	subq	%r8, %rsi
	/* Adjust destination which should be aligned now.  */
	subq	%r8, %rdi
	/* Adjust length.  */
	addq	%r8, %rdx

	/* Test if source and destination addresses will alias. If they
	   do the larger pipeline in large_memcpy_4x alleviated the
	   performance drop.  */

	/* ecx contains -(dst - src). not ecx will return dst - src - 1
	   which works for testing aliasing.  */
	notl	%ecx
	movq	%rdx, %r10
	testl	$(PAGE_SIZE - VEC_SIZE * 8), %ecx
	jz	L(large_memcpy_4x)

	/* r11 has __x86_shared_non_temporal_threshold.  Shift it left
	   by LOG_4X_MEMCPY_THRESH to get L(large_memcpy_4x) threshold.  */
	shlq	$LOG_4X_MEMCPY_THRESH, %r11
	cmp	%r11, %rdx
	jae	L(large_memcpy_4x)

	/* edx will store remainder size for copying tail.  */
	andl	$(PAGE_SIZE * 2 - 1), %edx
	/* r10 stores outer loop counter.  */
	shrq	$(LOG_PAGE_SIZE + 1), %r10
	/* Copy 4x VEC at a time from 2 pages.  */
	.p2align 4
L(loop_large_memcpy_2x_outer):
	/* ecx stores inner loop counter.  */
	movl	$(PAGE_SIZE / LARGE_LOAD_SIZE), %ecx
L(loop_large_memcpy_2x_inner):
	PREFETCH_ONE_SET (1, (%rsi), PREFETCHED_LOAD_SIZE)
	PREFETCH_ONE_SET (1, (%rsi), PREFETCHED_LOAD_SIZE * 2)
	PREFETCH_ONE_SET (1, (%rsi), PAGE_SIZE + PREFETCHED_LOAD_SIZE)
	PREFETCH_ONE_SET (1, (%rsi), PAGE_SIZE + PREFETCHED_LOAD_SIZE * 2)
	/* Load vectors from rsi.  */
	LOAD_ONE_SET ((%rsi), 0, %VMM(0), %VMM(1), %VMM(2), %VMM(3))
	LOAD_ONE_SET ((%rsi), PAGE_SIZE, %VMM(4), %VMM(5), %VMM(6), %VMM(7))
	subq	$-LARGE_LOAD_SIZE, %rsi
	/* Non-temporal store vectors to rdi.  */
	STORE_ONE_SET ((%rdi), 0, %VMM(0), %VMM(1), %VMM(2), %VMM(3))
	STORE_ONE_SET ((%rdi), PAGE_SIZE, %VMM(4), %VMM(5), %VMM(6), %VMM(7))
	subq	$-LARGE_LOAD_SIZE, %rdi
	decl	%ecx
	jnz	L(loop_large_memcpy_2x_inner)
	addq	$PAGE_SIZE, %rdi
	addq	$PAGE_SIZE, %rsi
	decq	%r10
	jne	L(loop_large_memcpy_2x_outer)
	sfence

	/* Check if only last 4 loads are needed.  */
	cmpl	$(VEC_SIZE * 4), %edx
	jbe	L(large_memcpy_2x_end)

	/* Handle the last 2 * PAGE_SIZE bytes.  */
L(loop_large_memcpy_2x_tail):
	/* Copy 4 * VEC a time forward with non-temporal stores.  */
	PREFETCH_ONE_SET (1, (%rsi), PREFETCHED_LOAD_SIZE)
	PREFETCH_ONE_SET (1, (%rdi), PREFETCHED_LOAD_SIZE)
	VMOVU	(%rsi), %VMM(0)
	VMOVU	VEC_SIZE(%rsi), %VMM(1)
	VMOVU	(VEC_SIZE * 2)(%rsi), %VMM(2)
	VMOVU	(VEC_SIZE * 3)(%rsi), %VMM(3)
	subq	$-(VEC_SIZE * 4), %rsi
	addl	$-(VEC_SIZE * 4), %edx
	VMOVA	%VMM(0), (%rdi)
	VMOVA	%VMM(1), VEC_SIZE(%rdi)
	VMOVA	%VMM(2), (VEC_SIZE * 2)(%rdi)
	VMOVA	%VMM(3), (VEC_SIZE * 3)(%rdi)
	subq	$-(VEC_SIZE * 4), %rdi
	cmpl	$(VEC_SIZE * 4), %edx
	ja	L(loop_large_memcpy_2x_tail)

L(large_memcpy_2x_end):
	/* Store the last 4 * VEC.  */
	VMOVU	-(VEC_SIZE * 4)(%rsi, %rdx), %VMM(0)
	VMOVU	-(VEC_SIZE * 3)(%rsi, %rdx), %VMM(1)
	VMOVU	-(VEC_SIZE * 2)(%rsi, %rdx), %VMM(2)
	VMOVU	-VEC_SIZE(%rsi, %rdx), %VMM(3)

	VMOVU	%VMM(0), -(VEC_SIZE * 4)(%rdi, %rdx)
	VMOVU	%VMM(1), -(VEC_SIZE * 3)(%rdi, %rdx)
	VMOVU	%VMM(2), -(VEC_SIZE * 2)(%rdi, %rdx)
	VMOVU	%VMM(3), -VEC_SIZE(%rdi, %rdx)
	VZEROUPPER_RETURN

	.p2align 4
L(large_memcpy_4x):
	/* edx will store remainder size for copying tail.  */
	andl	$(PAGE_SIZE * 4 - 1), %edx
	/* r10 stores outer loop counter.  */
	shrq	$(LOG_PAGE_SIZE + 2), %r10
	/* Copy 4x VEC at a time from 4 pages.  */
	.p2align 4
L(loop_large_memcpy_4x_outer):
	/* ecx stores inner loop counter.  */
	movl	$(PAGE_SIZE / LARGE_LOAD_SIZE), %ecx
L(loop_large_memcpy_4x_inner):
	/* Only one prefetch set per page as doing 4 pages give more
	   time for prefetcher to keep up.  */
	PREFETCH_ONE_SET (1, (%rsi), PREFETCHED_LOAD_SIZE)
	PREFETCH_ONE_SET (1, (%rsi), PAGE_SIZE + PREFETCHED_LOAD_SIZE)
	PREFETCH_ONE_SET (1, (%rsi), PAGE_SIZE * 2 + PREFETCHED_LOAD_SIZE)
	PREFETCH_ONE_SET (1, (%rsi), PAGE_SIZE * 3 + PREFETCHED_LOAD_SIZE)
	/* Load vectors from rsi.  */
	LOAD_ONE_SET ((%rsi), 0, %VMM(0), %VMM(1), %VMM(2), %VMM(3))
	LOAD_ONE_SET ((%rsi), PAGE_SIZE, %VMM(4), %VMM(5), %VMM(6), %VMM(7))
	LOAD_ONE_SET ((%rsi), PAGE_SIZE * 2, %VMM(8), %VMM(9), %VMM(10), %VMM(11))
	LOAD_ONE_SET ((%rsi), PAGE_SIZE * 3, %VMM(12), %VMM(13), %VMM(14), %VMM(15))
	subq	$-LARGE_LOAD_SIZE, %rsi
	/* Non-temporal store vectors to rdi.  */
	STORE_ONE_SET ((%rdi), 0, %VMM(0), %VMM(1), %VMM(2), %VMM(3))
	STORE_ONE_SET ((%rdi), PAGE_SIZE, %VMM(4), %VMM(5), %VMM(6), %VMM(7))
	STORE_ONE_SET ((%rdi), PAGE_SIZE * 2, %VMM(8), %VMM(9), %VMM(10), %VMM(11))
	STORE_ONE_SET ((%rdi), PAGE_SIZE * 3, %VMM(12), %VMM(13), %VMM(14), %VMM(15))
	subq	$-LARGE_LOAD_SIZE, %rdi
	decl	%ecx
	jnz	L(loop_large_memcpy_4x_inner)
	addq	$(PAGE_SIZE * 3), %rdi
	addq	$(PAGE_SIZE * 3), %rsi
	decq	%r10
	jne	L(loop_large_memcpy_4x_outer)
	sfence
	/* Check if only last 4 loads are needed.  */
	cmpl	$(VEC_SIZE * 4), %edx
	jbe	L(large_memcpy_4x_end)

	/* Handle the last 4  * PAGE_SIZE bytes.  */
L(loop_large_memcpy_4x_tail):
	/* Copy 4 * VEC a time forward with non-temporal stores.  */
	PREFETCH_ONE_SET (1, (%rsi), PREFETCHED_LOAD_SIZE)
	PREFETCH_ONE_SET (1, (%rdi), PREFETCHED_LOAD_SIZE)
	VMOVU	(%rsi), %VMM(0)
	VMOVU	VEC_SIZE(%rsi), %VMM(1)
	VMOVU	(VEC_SIZE * 2)(%rsi), %VMM(2)
	VMOVU	(VEC_SIZE * 3)(%rsi), %VMM(3)
	subq	$-(VEC_SIZE * 4), %rsi
	addl	$-(VEC_SIZE * 4), %edx
	VMOVA	%VMM(0), (%rdi)
	VMOVA	%VMM(1), VEC_SIZE(%rdi)
	VMOVA	%VMM(2), (VEC_SIZE * 2)(%rdi)
	VMOVA	%VMM(3), (VEC_SIZE * 3)(%rdi)
	subq	$-(VEC_SIZE * 4), %rdi
	cmpl	$(VEC_SIZE * 4), %edx
	ja	L(loop_large_memcpy_4x_tail)

L(large_memcpy_4x_end):
	/* Store the last 4 * VEC.  */
	VMOVU	-(VEC_SIZE * 4)(%rsi, %rdx), %VMM(0)
	VMOVU	-(VEC_SIZE * 3)(%rsi, %rdx), %VMM(1)
	VMOVU	-(VEC_SIZE * 2)(%rsi, %rdx), %VMM(2)
	VMOVU	-VEC_SIZE(%rsi, %rdx), %VMM(3)

	VMOVU	%VMM(0), -(VEC_SIZE * 4)(%rdi, %rdx)
	VMOVU	%VMM(1), -(VEC_SIZE * 3)(%rdi, %rdx)
	VMOVU	%VMM(2), -(VEC_SIZE * 2)(%rdi, %rdx)
	VMOVU	%VMM(3), -VEC_SIZE(%rdi, %rdx)
	VZEROUPPER_RETURN
#endif
