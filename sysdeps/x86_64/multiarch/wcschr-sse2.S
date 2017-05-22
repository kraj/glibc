/* wcschr optimized with SSE2.
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

#if IS_IN (libc)
# define __wcschr __wcschr_sse2

# ifdef SHARED
/* It doesn't make sense to send libc-internal wcschr calls through a PLT.
   The speedup we get from using AVX2 instructions is likely eaten away
   by the indirect call in the PLT.  */
#  undef libc_hidden_def
#  define libc_hidden_def(name) \
	.globl __GI___wcschr; __GI___wcschr = __wcschr_sse2
#  undef libc_hidden_weak
#  define libc_hidden_weak(name) \
	.weak __GI_wcschr; __GI_wcschr = __wcschr_sse2
# endif

# undef weak_alias
# define weak_alias(__wcschr, wcschr)
#endif

#include "../wcschr.S"
