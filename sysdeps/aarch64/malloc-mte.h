/* Definitions and macros for MTE (memory tagging) in malloc.
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
   <http://www.gnu.org/licenses/>.  */

#ifndef _AARCH64_MALLOC_MTE_H
#define _AARCH64_MALLOC_MTE_H

#include <stddef.h>
#include <sys/cdefs.h>

void *__libc_malloc_mte (size_t);
libc_hidden_proto (__libc_malloc_mte)
void *__libc_calloc_mte (size_t, size_t);
libc_hidden_proto (__libc_calloc_mte)
void *__libc_memalign_mte (size_t, size_t);
libc_hidden_proto (__libc_memalign_mte)
void *__libc_valloc_mte (size_t);
libc_hidden_proto (__libc_valloc_mte)
void *__libc_pvalloc_mte (size_t);
libc_hidden_proto (__libc_pvalloc_mte)
void *__libc_realloc_mte (void *, size_t);
libc_hidden_proto (__libc_realloc_mte)
void __libc_free_mte (void *);
libc_hidden_proto (__libc_free_mte)
size_t __malloc_usable_size_mte (void *);
libc_hidden_proto (__malloc_usable_size_mte)

#endif /* _AARCH64_MALLOC_MTE_H */
