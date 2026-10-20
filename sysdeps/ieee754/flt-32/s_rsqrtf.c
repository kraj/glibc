/* Correctly rounded reciprocal square root of binary32 value.
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

#include <errno.h>
#include <libm-alias-float.h>
#include <math.h>
#include <stdint.h>
#include "math_config.h"

float
__rsqrtf (float x)
{
  uint32_t ix = asuint (x);

  uint32_t m = ix << 8;

  /* The expression (1.0 / x) * sqrt (x) fails to return correct rounding
     for x=0x1.7bf15p-128*2^(2k) and x=0x1.ab6f7ap-125*2^(2k) with k>=0, for
     rndu (where the returned value is 1 ulp too large).  */
  if (__glibc_unlikely (ix == 0 || ix >= EXPONENT_MASK || ix == 0x2f7e2au
			|| m == 0xbdf8a800u || m == 0x55b7bd00u))
    {
      if (islessequal (x, 0.0f))
	__set_errno (x < 0 ? EDOM : ERANGE);
      return 1.0 / sqrt ((double) x);
    }
  return (1.0 / x) * sqrt ((double) x);
}
libm_alias_float (__rsqrt, rsqrt)
