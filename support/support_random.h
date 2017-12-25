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

typedef struct drand48_data support_random_state;

void support_random_seed (support_random_state *state, uint32_t seed);
void support_random_rseed (support_random_state *state);

uint32_t support_random_u32 (support_random_state *state);
void support_random_buf (support_random_state *state, void *buf,
			 size_t nbytes);

/* Scales the number NUMBER to the uniformly distributed closed internal
   [min, max].  */
static inline uint32_t
support_random_uniform_distribution (support_random_state *state,
				     uint32_t min, uint32_t max)
{
  uint32_t ret;
  uint32_t range = max - min;
  /* It assumes the input random number RANDOM range is as larger or equal
     than the RANGE, so the result will either returned or downscaled.  */
  if (range != UINT32_MAX)
    {
      uint32_t urange = range + 1;  /* range can be 0.  */
      uint32_t scaling = UINT32_MAX / urange;
      uint32_t past = urange * scaling;
      do
        ret = support_random_u32 (state);
      while (ret >= past);
      ret /= scaling;
    }
  else
    ret = support_random_u32 (state);
  return ret + min;
}

#endif
