/* Square root of _Float128 value, converting the result to _Float64x.
   Copyright (C) 2021-2025 Free Software Foundation, Inc.
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
#include <math-narrow.h>

/* math_ldbl.h defines _Float128 to long double for this directory,
   but when they are different, this function must be defined with
   _Float128 arguments to avoid defining an alias with an incompatible
   type.  */
#undef _Float128

#if __HAVE_FLOAT64X_LONG_DOUBLE && __HAVE_DISTINCT_FLOAT128
_Float64x
__f64xsqrtf128 (_Float128 x)
{
  NARROW_SQRT_ROUND_TO_ODD (x, _Float64x, union ieee854_long_double, l,
			    mantissa3);
}
libm_alias_float64x_float128 (sqrt)
#else
/* Defined as an alias of sqrtl.  */
#endif
