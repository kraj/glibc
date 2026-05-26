/* Code for ifunc resolvers for malloc: aarch64 version.
   Copyright (C) 2026 Free Software Foundation, Inc.
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

#if IS_IN (libc)

#include <init-arch.h>
#include <malloc-api.h>
#include <shlib-compat.h>

libc_ifunc_hidden (__libc_malloc, __libc_malloc_redirect,
		   __libc_malloc)
strong_alias (__libc_malloc_redirect, malloc)

libc_ifunc_hidden (__libc_calloc, __libc_calloc_redirect,
		   __libc_calloc)
weak_alias (__libc_calloc_redirect, calloc)

libc_ifunc_hidden (__libc_memalign, __libc_memalign_redirect,
		   __libc_memalign)
weak_alias (__libc_memalign_redirect, memalign)

libc_ifunc_hidden (__libc_valloc, __libc_valloc_redirect,
		   __libc_valloc)
weak_alias (__libc_valloc_redirect, valloc)

libc_ifunc_hidden (__libc_pvalloc, __libc_pvalloc_redirect,
		   __libc_pvalloc)
weak_alias (__libc_pvalloc_redirect, pvalloc)

libc_ifunc_hidden (__libc_realloc, __libc_realloc_redirect,
		   __libc_realloc)
strong_alias (__libc_realloc_redirect, realloc)

libc_ifunc_hidden (__libc_free, __libc_free_redirect,
		   __libc_free)
strong_alias (__libc_free_redirect, free)

libc_ifunc_hidden (__malloc_usable_size, __malloc_usable_size_redirect,
		   __malloc_usable_size)
weak_alias (__malloc_usable_size_redirect, malloc_usable_size)

libc_ifunc_hidden (__posix_memalign, __posix_memalign_redirect,
		   __posix_memalign)
weak_alias (__posix_memalign_redirect, posix_memalign)

libc_ifunc_hidden (__aligned_alloc, __aligned_alloc_redirect,
		   __aligned_alloc)
weak_alias (__aligned_alloc_redirect, aligned_alloc)

libc_ifunc_hidden (__free_sized, __free_sized_redirect,
		   __free_sized)
weak_alias (__free_sized_redirect, free_sized)

libc_ifunc_hidden (__free_aligned_sized, __free_aligned_sized_redirect,
		   __free_aligned_sized)
weak_alias (__free_aligned_sized_redirect, free_aligned_sized)

#endif /* IS_IN (libc) */

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_26)
compat_symbol (libc, __libc_free_redirect, cfree, GLIBC_2_0);
#endif
