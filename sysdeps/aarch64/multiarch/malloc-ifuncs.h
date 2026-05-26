/* Definitions for ifunc resolvers for malloc: aarch64 version.
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

#ifndef _AARCH64_MALLOC_IFUNCS_H
#define _AARCH64_MALLOC_IFUNCS_H

#define USE_MULTIARCH_MALLOC 1

#include <stddef.h>
#include <sys/cdefs.h>

/* Core implementations of malloc functions.  An ifunc resolver must
   use this implementations as a fallback option.  Other implementations
   may internally call these core function.  */
void *__libc_malloc (size_t);
libc_hidden_proto (__libc_malloc)
void *__libc_calloc (size_t, size_t);
libc_hidden_proto (__libc_calloc)
void *__libc_memalign (size_t, size_t);
libc_hidden_proto (__libc_memalign)
void *__libc_valloc (size_t);
libc_hidden_proto (__libc_valloc)
void *__libc_pvalloc (size_t);
libc_hidden_proto (__libc_pvalloc)
void *__libc_realloc (void *, size_t);
libc_hidden_proto (__libc_realloc)
void __libc_free (void *);
libc_hidden_proto (__libc_free)
size_t __malloc_usable_size (void *);
libc_hidden_proto (__malloc_usable_size)

/* For additions of POSIX.  */
int __posix_memalign (void **, size_t, size_t);
libc_hidden_proto (__posix_memalign)

/* For ISO C17.  */
void *__aligned_alloc (size_t, size_t);
libc_hidden_proto (__aligned_alloc)

/* For ISO C23.  */
void __free_sized (void *, size_t);
libc_hidden_proto (__free_sized)
void __free_aligned_sized (void *, size_t, size_t);
libc_hidden_proto (__free_aligned_sized)

#endif /* _AARCH64_MALLOC_IFUNCS_H */
