/* Euclidean distance function.  Float/Binary32 version.
   Copyright (C) 2012-2021 Free Software Foundation, Inc.
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

#include <math.h>
#include <math_private.h>
#include "math_config.h"
#include <libm-alias-finite.h>
#include <libm-alias-float.h>
#include <math-svid-compat.h>
#include <errno.h>

float
__hypotf (float x, float y)
{
   if (!isfinite(x) || !isfinite(y))
     {
       if ((isinf (x) || isinf (y))
	   && !issignalingf_inline (x) && !issignalingf_inline (y))
	 return INFINITY;
       return x + y;
     }

  float r = sqrt ((double) x * (double) x + (double) y * (double) y);
  if (!isfinite (r))
    __set_errno (ERANGE);
  return r;
}
strong_alias (__hypotf, __ieee754_hypotf)
#if LIBM_SVID_COMPAT
versioned_symbol (libm, __hypotf, hypotf, GLIBC_2_35);
libm_alias_float_other (__hypot, hypot)
#else
libm_alias_float (__hypot, hypot)
#endif
libm_alias_finite (__ieee754_hypotf, __hypotf)
