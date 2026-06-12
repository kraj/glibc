/* Scrub and reseed the AT_RANDOM bytes.  Linux version.
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
#include <not-cancel.h>
#include <sys/random.h>

/* The stack and pointer guards have been derived from the 16 AT_RANDOM
   bytes pointed to by DL_RANDOM.  Scrub them first, so the guards cannot be
   recovered even if the refill below fails, then refill them with fresh
   entropy unrelated to the guards so that getauxval (AT_RANDOM) keeps
   returning random bytes.  */
static inline void __attribute__ ((always_inline))
_dl_reseed_random (void **dl_random)
{
  if (*dl_random == NULL)
    return;
  memset (*dl_random, '\0', 16);
  __asm__ __volatile__ ("" : : "r" (*dl_random) : "memory");

  __getrandom_nocancel_nostatus_direct (*dl_random, 16, GRND_NONBLOCK);
  *dl_random = NULL;
}

#endif /* _DL_RESEED_RANDOM_H */
