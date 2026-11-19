/* Clear a block of memory for calloc.  Generic version.
   Copyright (C) 2024-2026 Free Software Foundation, Inc.
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

#include <string.h>

static __always_inline void *
clear_memory (void *p, unsigned long clearsize)
{
  INTERNAL_SIZE_T *d = (INTERNAL_SIZE_T *) p;
  INTERNAL_SIZE_T *end = (INTERNAL_SIZE_T *) ((uintptr_t) p + clearsize);

  /* Clearsize is the usable size of a chunk.  It is a multiple of SIZE_SZ
     and increments by MALLOC_ALIGNMENT.  Depending on the chunk layout,
     the minimum is either 2 * or 3 * SIZE_SZ.  */
  if (clearsize > 9 * SIZE_SZ)
    return memset (d, 0, clearsize);

#ifdef MALLOC_USER_SIZE_ALIGNMENT
  /* The 2-8 word sequence needs a usable size of at least 2 words.  */
  _Static_assert (MALLOC_USER_SIZE_ALIGNMENT <= SIZE_SZ * 2,
		  "MALLOC_USER_SIZE_ALIGNMENT too large for clear_memory");
  if (MALLOC_USER_SIZE_ALIGNMENT == SIZE_SZ * 2)
    {
      d[0] = 0;
      d[1] = 0;
      if (clearsize > 4 * SIZE_SZ)
	{
	  d[2] = 0;
	  d[3] = 0;
	  end[-4] = 0;
	  end[-3] = 0;
	}
      end[-2] = 0;
      end[-1] = 0;
      return d;
    }
#endif

  /* Clear 3-9 words using overlapping stores to reduce branching.  */
  d[0] = 0;
  d[1] = 0;
  d[2] = 0;
  end[-2] = 0;
  end[-1] = 0;
  if (clearsize > 5 * SIZE_SZ)
    {
      d[3] = 0;
      d[4] = 0;
      end[-4] = 0;
      end[-3] = 0;
    }

  return d;
}
