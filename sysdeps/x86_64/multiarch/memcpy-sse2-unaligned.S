/* memcpy with unaliged loads
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

#if IS_IN (libc)

#include <sysdep.h>

#include "asm-syntax.h"


ENTRY(__memcpy_sse2_unaligned)
	movq	%rdi, %rax
	movq	%rsi, %r11
	leaq	(%rdx,%rdx), %rcx
	subq	%rdi, %r11
	subq	%rdx, %r11
	cmpq	%rcx, %r11
	jb	L(overlapping)
	cmpq	$16, %rdx
	jbe	L(less_16)
	movdqu	(%rsi), %xmm8
	cmpq	$32, %rdx
	movdqu	%xmm8, (%rdi)
	movdqu	-16(%rsi,%rdx), %xmm8
	movdqu	%xmm8, -16(%rdi,%rdx)
	ja	.L31
L(return):
	ret
	.p2align 4,,10
	.p2align 4
.L31:
	movdqu	16(%rsi), %xmm8
	cmpq	$64, %rdx
	movdqu	%xmm8, 16(%rdi)
	movdqu	-32(%rsi,%rdx), %xmm8
	movdqu	%xmm8, -32(%rdi,%rdx)
	jbe	L(return)
	movdqu	32(%rsi), %xmm8
	cmpq	$128, %rdx
	movdqu	%xmm8, 32(%rdi)
	movdqu	-48(%rsi,%rdx), %xmm8
	movdqu	%xmm8, -48(%rdi,%rdx)
	movdqu	48(%rsi), %xmm8
	movdqu	%xmm8, 48(%rdi)
	movdqu	-64(%rsi,%rdx), %xmm8
	movdqu	%xmm8, -64(%rdi,%rdx)
	jbe	L(return)
	leaq	64(%rdi), %rcx
	addq	%rdi, %rdx
	andq	$-64, %rdx
	andq	$-64, %rcx
	movq	%rcx, %r11
	subq	%rdi, %r11
	addq	%r11, %rsi
	cmpq	%rdx, %rcx
	je	L(return)
	movq	%rsi, %r10
	subq	%rcx, %r10
	leaq	16(%r10), %r9
	leaq	32(%r10), %r8
	leaq	48(%r10), %r11
	.p2align 4,,10
	.p2align 4
L(loop):
	movdqu	(%rcx,%r10), %xmm8
	movdqa	%xmm8, (%rcx)
	movdqu	(%rcx,%r9), %xmm8
	movdqa	%xmm8, 16(%rcx)
	movdqu	(%rcx,%r8), %xmm8
	movdqa	%xmm8, 32(%rcx)
	movdqu	(%rcx,%r11), %xmm8
	movdqa	%xmm8, 48(%rcx)
	addq	$64, %rcx
	cmpq	%rcx, %rdx
	jne	L(loop)
	ret
L(overlapping):
	testq	%rdx, %rdx
	.p2align 4,,5
	je	L(return)
	movq	%rdx, %r9
	leaq	16(%rsi), %rcx
	leaq	16(%rdi), %r8
	shrq	$4, %r9
	movq	%r9, %r11
	salq	$4, %r11
	cmpq	%rcx, %rdi
	setae	%cl
	cmpq	%r8, %rsi
	setae	%r8b
	orl	%r8d, %ecx
	cmpq	$15, %rdx
	seta	%r8b
	testb	%r8b, %cl
	je	.L21
	testq	%r11, %r11
	je	.L21
	xorl	%ecx, %ecx
	xorl	%r8d, %r8d
.L7:
	movdqu	(%rsi,%rcx), %xmm8
	addq	$1, %r8
	movdqu	%xmm8, (%rdi,%rcx)
	addq	$16, %rcx
	cmpq	%r8, %r9
	ja	.L7
	cmpq	%r11, %rdx
	je	L(return)
.L21:
	movzbl	(%rsi,%r11), %ecx
	movb	%cl, (%rdi,%r11)
	addq	$1, %r11
	cmpq	%r11, %rdx
	ja	.L21
	ret
L(less_16):
	testb	$24, %dl
	jne	L(between_9_16)
	testb	$4, %dl
	.p2align 4,,5
	jne	L(between_5_8)
	testq	%rdx, %rdx
	.p2align 4,,2
	je	L(return)
	movzbl	(%rsi), %ecx
	testb	$2, %dl
	movb	%cl, (%rdi)
	je	L(return)
	movzwl	-2(%rsi,%rdx), %ecx
	movw	%cx, -2(%rdi,%rdx)
	ret
L(between_9_16):
	movq	(%rsi), %rcx
	movq	%rcx, (%rdi)
	movq	-8(%rsi,%rdx), %rcx
	movq	%rcx, -8(%rdi,%rdx)
	ret
L(between_5_8):
	movl	(%rsi), %ecx
	movl	%ecx, (%rdi)
	movl	-4(%rsi,%rdx), %ecx
	movl	%ecx, -4(%rdi,%rdx)
	ret
END(__memcpy_sse2_unaligned)

#endif
