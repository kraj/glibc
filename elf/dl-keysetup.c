/* Compute secret keys used for protection heuristics.
   Copyright (C) 2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <dl-keysetup.h>
#include <string.h>

enum { at_random_size = 16 };

#if __WORDSIZE == 64
enum { sha256_output_size = 32 };
static void compute_sha256_of_random (const void *random, void *result);
#endif

void
__compute_keys (const void *random, struct key_setup *result)
{
#if __WORDSIZE == 32
  _Static_assert (sizeof (*result) == at_random_size,
                  "no key expansion required");
  memcpy (random, result, sizeof (result));
#else
  /* We use SHA-256 to expand the 16 bytes of randomness into 32
     bytes, so that it is hard to guess the remaining keys once a
     subset of them is known.  */
  _Static_assert (sizeof (*result) == sha256_output_size,
                  "SHA-256 provides required size");
  compute_sha256_of_random (random, result);
#endif

  /* Prevent leakage of the stack canary through a read buffer
     overflow of a NUL-terminated string.  */
  *(char *) &result->stack = '\0';

  /* Clear the lowest three bits in the heap header guard value, so
     that the flag bits remain unchanged.  */
  result->heap_header <<= 3;
}

#if __WORDSIZE == 64

#pragma GCC visibility push (hidden)

/* Avoid symbol collisions with libcrypt.  */
#define __sha256_process_block __dl_sha256_process_block
#define __sha256_init_ctx __dl_sha256_init_ctx
#define __sha256_process_bytes __dl_sha256_process_bytes
#define __sha256_finish_ctx __dl_sha256_finish_ctx

#include "../crypt/sha256.h"
#include "../crypt/sha256.c"

#pragma GCC visibility pop

static void
compute_sha256_of_random (const void *random, void *result)
{
  struct sha256_ctx ctx;
  __sha256_init_ctx (&ctx);
  __sha256_process_bytes (random, at_random_size, &ctx);
  __sha256_finish_ctx (&ctx, result);
}
#endif
