/* Scrub and reseed the kernel-provided random bytes.  Generic version.
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

#ifndef _DL_RESEED_RANDOM_H
#define _DL_RESEED_RANDOM_H

#include <string.h>

static inline void __attribute__ ((always_inline))
_dl_reseed_random (void **dl_random)
{
  if (*dl_random == NULL)
    return;
  memset (*dl_random, '\0', 16);
  __asm__ __volatile__ ("" : : "r" (*dl_random) : "memory");
  *dl_random = NULL;
}

#endif /* _DL_RESEED_RANDOM_H */
