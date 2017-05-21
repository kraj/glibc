/* Multiple versions of wmemset_chk
   All versions must be listed in ifunc-impl-list.c.
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
#include <shlib-compat.h>
#include <init-arch.h>

/* Define multiple versions only for the definition in libc. */
#if IS_IN (libc)
# ifdef SHARED
ENTRY(__wmemset_chk)
	.type	__wmemset_chk, @gnu_indirect_function
	LOAD_RTLD_GLOBAL_RO_RDX
	lea	__wmemset_chk_sse2_unaligned(%rip), %RAX_LP
	HAS_ARCH_FEATURE (AVX2_Usable)
	jz	1f
	lea	__wmemset_chk_avx2_unaligned(%rip), %RAX_LP
	HAS_ARCH_FEATURE (Prefer_No_AVX512)
	jnz	1f
	HAS_ARCH_FEATURE (Prefer_No_VZEROUPPER)
	jnz	1f
	HAS_ARCH_FEATURE (AVX512F_Usable)
	jz	1f
	lea	__wmemset_chk_avx512_unaligned(%rip), %RAX_LP
1:	ret
END(__wmemset_chk)
# else
#  include "../wmemset_chk.S"
# endif
#endif
