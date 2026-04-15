/* Definitions for ifunc resolvers for malloc: generic version.
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

#ifndef _GENERIC_MALLOC_IFUNCS_H
#define _GENERIC_MALLOC_IFUNCS_H

#if HAVE_IFUNC

#include <stddef.h>
#include <sys/cdefs.h>
#include <shlib-compat.h>

/* Core implementations of malloc functions.  An ifunc resolver must
   use this implementations as a fallback option.  Other implementations
   may internally call these core function.  */
void *__libc_malloc_core (size_t);
libc_hidden_proto (__libc_malloc_core)
void *__libc_calloc_core (size_t, size_t);
libc_hidden_proto (__libc_calloc_core)
void *__libc_memalign_core (size_t, size_t);
libc_hidden_proto (__libc_memalign_core)
void *__libc_valloc_core (size_t);
libc_hidden_proto (__libc_valloc_core)
void *__libc_pvalloc_core (size_t);
libc_hidden_proto (__libc_pvalloc_core)
void *__libc_realloc_core (void *, size_t);
libc_hidden_proto (__libc_realloc_core)
void __libc_free_core (void *);
libc_hidden_proto (__libc_free_core)
size_t __malloc_usable_size_core (void *);
libc_hidden_proto (__malloc_usable_size_core)

/* For additions of POSIX.  */
int __posix_memalign_core (void **, size_t, size_t);
libc_hidden_proto (__posix_memalign_core)

/* For ISO C17.  */
void *__aligned_alloc_core (size_t, size_t);
libc_hidden_proto (__aligned_alloc_core)

/* For ISO C23.  */
void __free_sized_core (void *, size_t);
libc_hidden_proto (__free_sized_core)
void __free_aligned_sized_core (void *, size_t, size_t);
libc_hidden_proto (__free_aligned_sized_core)

/* Macros for defining ifunc resolvers for malloc functions.  */
#define IFUNC_RESOLVER_NAME(fn) fn ## _resolver
#define STR(x) #x
#define XSTR(x) STR(x)
#define IFUNC_PROTO(fn) \
  __typeof (fn ## _core) fn \
  __attribute__ ((ifunc (XSTR(IFUNC_RESOLVER_NAME(fn)))))
#define IFUNC_RESOLVER(fn, ...) \
  static __attribute_used__ \
  __typeof (fn ## _core) *IFUNC_RESOLVER_NAME(fn) (__VA_ARGS__)

#endif /* HAVE_IFUNC */

#endif /* _GENERIC_MALLOC_IFUNCS_H */
