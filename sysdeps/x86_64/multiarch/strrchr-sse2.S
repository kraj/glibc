/* strrchr optimized with SSE2.
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
# define strrchr __strrchr_sse2

# ifdef SHARED
#  undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strrchr calls through a PLT.
   The speedup we get from using SSE4.2 instruction is likely eaten away
   by the indirect call in the PLT.  */
#  define libc_hidden_builtin_def(name) \
	.globl __GI_strrchr; __GI_strrchr = __strrchr_sse2
# endif

# undef weak_alias
# define weak_alias(strrchr, rindex)
#endif

#include "../strrchr.S"
