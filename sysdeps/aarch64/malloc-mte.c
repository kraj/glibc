/* Implementation for MTE (memory tagging) in malloc.
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

#include "malloc-mte.h"
#include "aarch64-mte.h"

#include <malloc-ifuncs.h>
#include <errno.h>

#define TAG_MEM(ptr, tagfun) __glibc_unlikely (ptr == NULL) ? NULL : ({ \
  size_t size = __malloc_usable_size_core (ptr); \
  tagfun (__mte_new_tag (ptr), size); \
})

void *__libc_malloc_mte (size_t bytes)
{
  void *untagged = __libc_malloc_core (bytes);
  return TAG_MEM (untagged, __mte_tag_region);
}
libc_hidden_def (__libc_malloc_mte)

void *__libc_calloc_mte (size_t n, size_t elem_size)
{
  /* We use core malloc instead of calloc because we can
     take advantage of MTE to zero memory region.  */
  void *untagged = __libc_malloc_core (n * elem_size);
  return TAG_MEM (untagged, __mte_tag_region_zero);
}
libc_hidden_def (__libc_calloc_mte)

void *__libc_memalign_mte (size_t alignment, size_t bytes)
{
  void *untagged = __libc_memalign_core (alignment, bytes);
  return TAG_MEM (untagged, __mte_tag_region);
}
libc_hidden_def (__libc_memalign_mte)

void *__libc_valloc_mte (size_t bytes)
{
  void *untagged = __libc_valloc_core (bytes);
  return TAG_MEM (untagged, __mte_tag_region);
}
libc_hidden_def (__libc_valloc_mte)

void *__libc_pvalloc_mte (size_t bytes)
{
  void *untagged = __libc_pvalloc_core (bytes);
  return TAG_MEM (untagged, __mte_tag_region);
}
libc_hidden_def (__libc_pvalloc_mte)

/* See malloc.c for details.  */
#ifndef REALLOC_ZERO_BYTES_FREES
#define REALLOC_ZERO_BYTES_FREES 1
#endif

void *__libc_realloc_mte (void *tagged_oldmem, size_t bytes)
{
  /* Quick check: realloc of null is supposed to be same as malloc.  */
  if (tagged_oldmem == NULL)
    return __libc_malloc_mte (bytes);

#if REALLOC_ZERO_BYTES_FREES
  /* Quick check: realloc with 0 size is supposed to be same as free.  */
  if (bytes == 0)
    {
      __libc_free_mte (tagged_oldmem);
      return NULL;
    }
#endif

  /* Bad size, old memory remains unchanged.  */
  if (bytes > PTRDIFF_MAX)
    {
      __set_errno (ENOMEM);
      return NULL;
    }

  /* At this point we untag oldmem allocation.  */
  void *untagged_oldmem = __mte_clear_tag (tagged_oldmem);

  /* Mark the chunk as belonging to the library again.  */
  size_t size_old = __malloc_usable_size_core (untagged_oldmem);
  untagged_oldmem = __mte_tag_region (untagged_oldmem, size_old);

  /* Call realloc core.  */
  void *untagged_newmem = __libc_realloc_core (untagged_oldmem, bytes);
  if (untagged_newmem == NULL)
    return NULL;
  size_t size_new = __malloc_usable_size_core (untagged_newmem);

  /* If realloc core returns old pointer, we need re-tag it.  */
  if (size_new == size_old && untagged_newmem == untagged_oldmem)
    return __mte_tag_region (tagged_oldmem, size_new);

  /* Otherwise, assign new tag.  */
  void *tagged_newmem = __mte_new_tag (untagged_newmem);
  return __mte_tag_region (tagged_newmem, size_new);
}
libc_hidden_def (__libc_realloc_mte)

void __libc_free_mte (void *tagged)
{
  if (__glibc_unlikely (tagged == NULL))
    return;
  /* Mark the chunk as belonging to the library again.  */
  void *untagged = __mte_clear_tag (tagged);
  size_t size = __malloc_usable_size_core (untagged);
  untagged = __mte_tag_region (untagged, size);
  /* Call free core.  */
  __libc_free_core (untagged);
}
libc_hidden_def (__libc_free_mte)

size_t __malloc_usable_size_mte (void *tagged)
{
  /* Clear logical tag only to allow accessing internal malloc
    structures via offset from this pointer.  */
  void *untagged = __mte_clear_tag (tagged);
  return __malloc_usable_size_core (untagged);
}
libc_hidden_def (__malloc_usable_size_mte)

int __posix_memalign_mte (void **memptr, size_t alignment, size_t size)
{
  int err = __posix_memalign_core (memptr, alignment, size);
  if (err != 0)
    return err;
  *memptr = TAG_MEM (*memptr, __mte_tag_region);
  return err;
}
libc_hidden_def (__posix_memalign_mte)

void *__aligned_alloc_mte (size_t alignment, size_t bytes)
{
  void *untagged = __aligned_alloc_core (alignment, bytes);
  return TAG_MEM (untagged, __mte_tag_region);
}
libc_hidden_def (__aligned_alloc_mte)

void __free_sized_mte (void *tagged, size_t size)
{
  if (__glibc_unlikely (tagged == NULL))
    return;
  /* Mark the chunk as belonging to the library again.  */
  void *untagged = __mte_clear_tag (tagged);
  size_t int_size = __malloc_usable_size_core (untagged);
  untagged = __mte_tag_region (untagged, int_size);
  /* Call core function.  */
  __free_sized_core (untagged, size);
}
libc_hidden_def (__free_sized_mte)

void __free_aligned_sized_mte (void *tagged, __attribute_maybe_unused__ size_t alignment, __attribute_maybe_unused__ size_t size)
{
  if (__glibc_unlikely (tagged == NULL))
    return;
  /* Mark the chunk as belonging to the library again.  */
  void *untagged = __mte_clear_tag (tagged);
  size_t int_size = __malloc_usable_size_core (untagged);
  untagged = __mte_tag_region (untagged, int_size);
  /* Call core function.  */
  __free_aligned_sized_core (untagged, alignment, size);
}
libc_hidden_def (__free_aligned_sized_mte)
