/* Return tangent of pi * X.
   Copyright (C) 2024-2025 Free Software Foundation, Inc.
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
#include <math.h>
#include <math-underflow.h>

FLOAT
M_DECL_FUNC (__tanpi) (FLOAT x)
{
  if (isless (M_FABS (x), M_EPSILON))
    {
      FLOAT ret = M_MLIT (M_PI) * x;
      math_check_force_underflow (ret);
      return ret;
    }
  if (__glibc_unlikely (isinf (x)))
    __set_errno (EDOM);
  FLOAT y = x - M_LIT (2.0) * M_SUF (round) (M_LIT (0.5) * x);
  FLOAT absy = M_FABS (y);
  if (absy == M_LIT (0.0))
    /* For even integers, return +0 if positive and -0 if negative (so
       matching sinpi(x)/cospi(x)).  */
    return M_COPYSIGN (M_LIT (0.0), x);
  else if (absy == M_LIT (1.0))
    /* For odd integers, return -0 if positive and +0 if negative (so
       matching sinpi(x)/cospi(x)).  */
    return M_COPYSIGN (M_LIT (0.0), -x);
  else if (absy == M_LIT (0.5))
    {
      /* Return infinity with positive sign for an even integer + 0.5
	 and negative sign for an odd integer + 0.5 (so matching
	 sinpi(x)/cospi(x)).  */
      __set_errno (ERANGE);
      return 1.0 / M_COPYSIGN (M_LIT (0.0), y);
    }
  /* Now we only care about the value of X mod 1, not mod 2.  */
  else if (isgreater (absy, 0.5))
    {
      y -= M_COPYSIGN (M_LIT (1.0), y);
      absy = M_FABS (y);
    }
  if (islessequal (absy, M_LIT (0.25)))
    return M_SUF (__tan) (M_MLIT (M_PI) * y);
  else
    return M_COPYSIGN (M_LIT (1.0)
		       / M_SUF (__tan) (M_MLIT (M_PI) * (M_LIT (0.5) - absy)),
		       y);
}
declare_mgen_alias (__tanpi, tanpi);
