/* Function for pseudo-random number generation.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

#ifndef SUPPORT_MT_RAND_H
#define SUPPORT_MT_RAND_H

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* Obtain a random seed at BUF with size of LEN from system random device.
   It will used getrandom or '/dev/urandom' device case getrandom fails.  */
int random_seed (void *buf, size_t len);

/* A Mersenne Twister implementation for both uint32_t and uint64_t aimed for
   fast pseudo-random number generation where rand() is not suffice (due
   mainly low entropy).

   The usual way to use is:

   uint32_t seed;
   random_seed (&seed, sizeof (uint32_t));

   mt19937_32 mt;
   mt32_seed (&mt, seed);

   uint32_t random_number = mt32_rand (&mt);

   If seed is 0 the default one (5489u) is used instead.  Usually the seed
   should be obtained for a more robust random generation (getrandom or
   from /dev/{u}random).  */

enum {
  MT32_STATE_SIZE = 624,
  MT64_STATE_SIZE = 312
};

struct mt19937_32
{
  uint32_t mt[MT32_STATE_SIZE];
  size_t p;
};

struct mt19937_64
{
  uint64_t mt[MT64_STATE_SIZE];
  size_t p;
};

/* Initialize the mersenne twister STATE with SEED.  If seed is zero the
   default seed is used (5489u).  */
void mt32_seed (struct mt19937_32 *state, uint32_t seed);
void mt64_seed (struct mt19937_64 *state, uint64_t seed);
/* Output a pseudo-number from mersenned twister STATE.  */
uint32_t mt32_rand (struct mt19937_32 *state);
uint64_t mt64_rand (struct mt19937_64 *state);

/* Scales the number NUMBER to the uniformly distributed closed internal
   [min, max].  */
static inline int32_t
uniform_uint32_distribution (int32_t random, uint32_t min, uint32_t max)
{
  assert (max >= min);
  uint32_t range = max - min;
  /* It assumed that the input random number RANDOM is as larger or equal
     than the RANGE, so the result will always be downscaled.  */
  if (range != UINT32_MAX)
    {
      uint32_t urange = range + 1;  /* range can be 0.  */
      uint32_t scaling = UINT32_MAX / urange;
      random /= scaling;
    }
  return random + min;
}

/* Scales the number NUMBER to the uniformly distributed closed internal
   [min, max].  */
static inline uint64_t
uniform_uint64_distribution (uint64_t random, uint64_t min, uint64_t max)
{
  assert (max >= min);
  uint64_t range = max - min;
  /* It assumed that the input random number RANDOM is as larger or equal
     than the RANGE, so the result will always be downscaled.  */
  if (range != UINT64_MAX)
    {
      uint64_t urange = range + 1;  /* range can be 0.  */
      uint64_t scaling = UINT64_MAX / urange;
      random /= scaling;
    }
  return random + min;
}

#endif
