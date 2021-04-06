/* Euclidean distance function.  Long Double/Binary128 version.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

/* This implementation is based on 'An Improved Algorithm for hypot(a,b)' by
   Carlos F. Borges [1] using the MyHypot3 with the following changes:

   - Handle qNaN and sNaN.
   - Tune the 'widely varying operands' to avoid spurious underflow
     due the multiplication and fix the return value for upwards
     rounding mode.
   - Handle required underflow exception for subnormal results.

   [1] https://arxiv.org/pdf/1904.09481.pdf  */

#include <math.h>
#include <math_private.h>
#include <math-narrow-eval.h>
#include <math-underflow.h>
#include <libm-alias-finite.h>

/* sqrt (LDBL_EPSILON / 2.0)  */
#define SQRT_EPS_DIV_2      L(0x1.6a09e667f3bcc908b2fb1366ea95p-57)
/* DBL_MIN / (sqrt (LDBL_EPSILON / 2.0))   */
#define LDBL_MIN_THRESHOLD  L(0x1.6a09e667f3bcc908b2fb1366ea96p-16326)
/* eps (long double) *(sqrt (LDBL_MIN)  */
#define SCALE               L(0x1p-8303)
/* 1 / eps (sqrt (LDBL_MIN)  */
#define INV_SCALE           L(0x1p+8303)
/* sqrt (LDBL_MAX)  */
#define SQRT_LDBL_MAX       L(0x1.6a09e667f3bcc908b2fb1366ea95p+8191)
/* sqrt (LDBL_MIN)  */
#define SQRT_LDBL_MIN       L(0x1p-8191)

/* Hypot kernel. The inputs must be adjusted so that ax >= ay >= 0
   and squaring ax, ay and (ax - ay) does not overflow or underflow.  */
static inline _Float128
kernel (_Float128 ax, _Float128 ay)
{
  _Float128 t1, t2;
  _Float128 h = sqrtl (ax * ax + ay * ay);
  if (h == L(0.0))
    return h;
  if (h <= L(2.0) * ay)
    {
      _Float128 delta = h - ay;
      t1 = ax * (L(2.0) * delta - ax);
      t2 = (delta - L(2.0) * (ax - ay)) * delta;
    }
  else
    {
      _Float128 delta = h - ax;
      t1 = L(2.0) * delta * (ax - L(2.0) * ay);
      t2 = (L(4.0) * delta - ay) * ay + delta * delta;
    }

  h -= (t1 + t2) / (L(2.0) * h);
  return h;
}

_Float128
__ieee754_hypotl(_Float128 x, _Float128 y)
{
  if (!isfinite(x) || !isfinite(y))
    {
      if ((isinf (x) || isinf (y))
	  && !issignaling (x) && !issignaling (y))
	return INFINITY;
      return x + y;
    }

  x = fabsl (x);
  y = fabsl (y);

  _Float128 ax = x < y ? y : x;
  _Float128 ay = x < y ? x : y;

  /* If ax is huge, scale both inputs down.  */
  if (__glibc_unlikely (ax > SQRT_LDBL_MAX))
    {
      if (ay <= ax * SQRT_EPS_DIV_2)
	return ay == L(0.0) ? ax : ax + LDBL_TRUE_MIN;

      return kernel (ax * SCALE, ay * SCALE) / SCALE;
    }

  /* Common case: ax is not huge and ay is not tiny.  */
  if (__glibc_unlikely (ay < SQRT_LDBL_MIN))
    {
      /* Widely varying operands.  The LDBL_MIN_THRESHOLD check is used to avoid
	 a spurious underflow from the multiplication.  */
      if (__glibc_unlikely (ax >= LDBL_MIN_THRESHOLD && ay <= ax * SQRT_EPS_DIV_2))
	return ay == L(0.0) ? ax : ax + LDBL_TRUE_MIN;

      ax = math_narrow_eval (kernel (ax / SCALE, ay / SCALE) * SCALE);
      math_check_force_underflow_nonneg (ax);
      return ax;
    }

  /* Common case: ax is not huge and ay is not tiny.  */
  if (ay <= ax * SQRT_EPS_DIV_2)
    return ay == L(0.0) ? ax : ax + LDBL_TRUE_MIN;

  return kernel (ax, ay);
}
libm_alias_finite (__ieee754_hypotl, __hypotl)
