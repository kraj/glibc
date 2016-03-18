/* Multiple versions of mempcpy
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2010-2016 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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
#include <init-arch.h>

/* Define multiple versions only for the definition in lib and for
   DSO.  In static binaries we need mempcpy before the initialization
   happened.  */
#if defined SHARED && IS_IN (libc)
ENTRY(__mempcpy)
	.type	__mempcpy, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
#ifdef HAVE_AVX512_ASM_SUPPORT
	HAS_ARCH_FEATURE (AVX512F_Usable)
	jz	1f
	HAS_ARCH_FEATURE (Prefer_No_VZEROUPPER)
	jz	1f
	lea	__mempcpy_avx512_no_vzeroupper(%rip), %RAX_LP
	ret
#endif
1:	lea	__mempcpy_avx_unaligned_erms(%rip), %RAX_LP
	HAS_ARCH_FEATURE (AVX_Fast_Unaligned_Load)
	jz	 L(Fast_Unaligned_Load)
	HAS_ARCH_FEATURE (Hybrid_ERMS)
	jnz	2f
	lea	__mempcpy_avx_unaligned(%rip), %RAX_LP
	ret
L(Fast_Unaligned_Load):
	lea	__mempcpy_sse2_unaligned_erms(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Fast_Unaligned_Load)
	jz	L(SSE2)
	HAS_ARCH_FEATURE (Hybrid_ERMS)
	jnz	2f
	lea	__mempcpy_sse2_unaligned_2(%rip), %RAX_LP
	ret
L(SSE2):
	lea	__mempcpy_sse2(%rip), %RAX_LP
	HAS_CPU_FEATURE (SSSE3)
	jz	2f
	lea	__mempcpy_ssse3_back(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Fast_Copy_Backward)
	jnz	2f
	lea	__mempcpy_ssse3(%rip), %RAX_LP
2:	ret
END(__mempcpy)

# undef ENTRY
# define ENTRY(name) \
	.type __mempcpy_sse2, @function; \
	.p2align 4; \
	.globl __mempcpy_sse2; \
	.hidden __mempcpy_sse2; \
	__mempcpy_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __mempcpy_sse2, .-__mempcpy_sse2

# undef ENTRY_CHK
# define ENTRY_CHK(name) \
	.type __mempcpy_chk_sse2, @function; \
	.globl __mempcpy_chk_sse2; \
	.p2align 4; \
	__mempcpy_chk_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END_CHK
# define END_CHK(name) \
	cfi_endproc; .size __mempcpy_chk_sse2, .-__mempcpy_chk_sse2

# undef libc_hidden_def
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal mempcpy calls through a PLT.
   The speedup we get from using SSSE3 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_def(name) \
	.globl __GI_mempcpy; __GI_mempcpy = __mempcpy_sse2
# define libc_hidden_builtin_def(name) \
	.globl __GI___mempcpy; __GI___mempcpy = __mempcpy_sse2
#endif

#include "../mempcpy.S"
