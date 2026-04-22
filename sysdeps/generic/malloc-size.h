/* Size-related definitions for malloc: generic version.
   Copyright (C) 2021-2026 Free Software Foundation, Inc.
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

#ifndef _GENERIC_MALLOC_SIZE_H
#define _GENERIC_MALLOC_SIZE_H

#include <malloc-chunk.h>
#include <stdint.h>
#include <sys/cdefs.h>

/* The smallest size we can malloc is an aligned minimal chunk.  */
#define MINSIZE \
  (unsigned long)(((MIN_CHUNK_SIZE + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))

/* Pad request bytes into a usable size -- internal version.  Note: This must
   be a macro that evaluates to a compile time constant if passed a literal
   constant.  */
#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)

/* Check if REQ overflows when padded and aligned and if the resulting
   value is less than PTRDIFF_T.  Returns the requested size or
   MINSIZE in case the value is less than MINSIZE, or SIZE_MAX if any
   of the previous checks fail.  */
static __always_inline __attribute_maybe_unused__ size_t
checked_request2size (size_t req) __nonnull (1)
{
  if (__glibc_unlikely (req > PTRDIFF_MAX))
    return SIZE_MAX;
  return request2size (req);
}

/* Like chunksize, but do not mask SIZE_BITS.  */
#define chunksize_nomask(p) ((p)->mchunk_size)

/* Get size, ignoring use bits.  */
#define chunksize(p) (chunksize_nomask (p) & ~(SIZE_BITS))

/* This is the size of the real usable data in the chunk.  Not valid for
   dumped heap chunks.  */
#define memsize(p) (chunksize (p) - CHUNK_HDR_SZ + SIZE_SZ)

#endif /* _GENERIC_MALLOC_SIZE_H */
