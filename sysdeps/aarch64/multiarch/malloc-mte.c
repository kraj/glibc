/* Implementation for MTE (memory tagging) wrappers in malloc.
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

#include <malloc-ifuncs.h>

void *__libc_malloc_mte (size_t bytes)
{
  return __libc_malloc (bytes);
}
libc_hidden_def (__libc_malloc_mte)

void *__libc_calloc_mte (size_t n, size_t elem_size)
{
  return __libc_calloc (n, elem_size);
}
libc_hidden_def (__libc_calloc_mte)

void *__libc_memalign_mte (size_t alignment, size_t bytes)
{
  return __libc_memalign (alignment, bytes);
}
libc_hidden_def (__libc_memalign_mte)

void *__libc_valloc_mte (size_t bytes)
{
  return __libc_valloc (bytes);
}
libc_hidden_def (__libc_valloc_mte)

void *__libc_pvalloc_mte (size_t bytes)
{
  return __libc_pvalloc (bytes);
}
libc_hidden_def (__libc_pvalloc_mte)

void *__libc_realloc_mte (void *oldmem, size_t bytes)
{
  return __libc_realloc (oldmem, bytes);
}
libc_hidden_def (__libc_realloc_mte)

void __libc_free_mte (void *mem)
{
  __libc_free (mem);
}
libc_hidden_def (__libc_free_mte)

size_t __malloc_usable_size_mte (void *m)
{
  return __malloc_usable_size (m);
}
libc_hidden_def (__malloc_usable_size_mte)

int __posix_memalign_mte (void **memptr, size_t alignment, size_t size)
{
  return __posix_memalign (memptr, alignment, size);
}
libc_hidden_def (__posix_memalign_mte)

void *__aligned_alloc_mte (size_t alignment, size_t bytes)
{
  return __aligned_alloc (alignment, bytes);
}
libc_hidden_def (__aligned_alloc_mte)

void __free_sized_mte (void *ptr, size_t size)
{
  __free_sized (ptr, size);
}
libc_hidden_def (__free_sized_mte)

void __free_aligned_sized_mte (void *ptr, size_t alignment, size_t size)
{
  __free_aligned_sized (ptr, alignment, size);
}
libc_hidden_def (__free_aligned_sized_mte)
