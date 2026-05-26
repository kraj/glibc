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

#include <stdint.h>
#include <malloc-ifuncs.h>

/* Macros for defining ifunc resolvers for malloc functions.  */
#define IFUNC_RESOLVER_NAME(fn) fn ## _resolver
#define STR(x) #x
#define XSTR(x) STR(x)
#define IFUNC_PROTO(fn) \
  __typeof (fn) fn ## _ifunc \
  __attribute__ ((ifunc (XSTR(IFUNC_RESOLVER_NAME(fn)))))
#define IFUNC_RESOLVER(fn, ...) \
  static __attribute_used__ \
  __typeof (fn) *IFUNC_RESOLVER_NAME(fn) (__VA_ARGS__)

IFUNC_PROTO (__libc_malloc);
IFUNC_RESOLVER (__libc_malloc, uint64_t arg0, uint64_t arg1[])
{
  return __libc_malloc;
}
strong_alias (__libc_malloc_ifunc, malloc)

IFUNC_PROTO (__libc_calloc);
IFUNC_RESOLVER (__libc_calloc, uint64_t arg0, uint64_t arg1[])
{
  return __libc_calloc;
}
weak_alias (__libc_calloc_ifunc, calloc)

IFUNC_PROTO (__libc_memalign);
IFUNC_RESOLVER (__libc_memalign, uint64_t arg0, uint64_t arg1[])
{
  return __libc_memalign;
}
weak_alias (__libc_memalign_ifunc, memalign)

IFUNC_PROTO (__libc_valloc);
IFUNC_RESOLVER (__libc_valloc, uint64_t arg0, uint64_t arg1[])
{
  return __libc_valloc;
}
weak_alias (__libc_valloc_ifunc, valloc)

IFUNC_PROTO (__libc_pvalloc);
IFUNC_RESOLVER (__libc_pvalloc, uint64_t arg0, uint64_t arg1[])
{
  return __libc_pvalloc;
}
weak_alias (__libc_pvalloc_ifunc, pvalloc)

IFUNC_PROTO (__libc_realloc);
IFUNC_RESOLVER (__libc_realloc, uint64_t arg0, uint64_t arg1[])
{
  return __libc_realloc;
}
strong_alias (__libc_realloc_ifunc, realloc)

IFUNC_PROTO (__libc_free);
IFUNC_RESOLVER (__libc_free, uint64_t arg0, uint64_t arg1[])
{
  return __libc_free;
}
strong_alias (__libc_free_ifunc, free)

IFUNC_PROTO (__malloc_usable_size);
IFUNC_RESOLVER (__malloc_usable_size, uint64_t arg0, uint64_t arg1[])
{
  return __malloc_usable_size;
}
weak_alias (__malloc_usable_size_ifunc, malloc_usable_size)

IFUNC_PROTO (__posix_memalign);
IFUNC_RESOLVER (__posix_memalign, uint64_t arg0, uint64_t arg1[])
{
  return __posix_memalign;
}
weak_alias (__posix_memalign_ifunc, posix_memalign)

IFUNC_PROTO (__aligned_alloc);
IFUNC_RESOLVER (__aligned_alloc, uint64_t arg0, uint64_t arg1[])
{
  return __aligned_alloc;
}
weak_alias (__aligned_alloc_ifunc, aligned_alloc)

IFUNC_PROTO (__free_sized);
IFUNC_RESOLVER (__free_sized, uint64_t arg0, uint64_t arg1[])
{
  return __free_sized;
}
weak_alias (__free_sized_ifunc, free_sized)

IFUNC_PROTO (__free_aligned_sized);
IFUNC_RESOLVER (__free_aligned_sized, uint64_t arg0, uint64_t arg1[])
{
  return __free_aligned_sized;
}
weak_alias (__free_aligned_sized_ifunc, free_aligned_sized)

#endif /* IS_IN (libc) */
