/* Euclidean distance function.  Long Double/Binary96 version.
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
#define SQRT_EPS_DIV_2      0x8p-35L
/* DBL_MIN / (sqrt (LDBL_EPSILON / 2.0))   */
#define LDBL_MIN_THRESHOLD  0x8p-16353L
/* eps (long double) * sqrt (LDBL_MIN)  */
#define SCALE               0x8p-8257L
/* 1 / eps (sqrt (LDBL_MIN)  */
#define INV_SCALE           0x8p+8251L
/* sqrt (LDBL_MAX)  */
#define SQRT_LDBL_MAX       0xb.504f333f9de6484p+8188L
/* sqrt (LDBL_MIN)  */
#define SQRT_LDBL_MIN       0x8p-8194L

/* Hypot kernel. The inputs must be adjusted so that ax >= ay >= 0
   and squaring ax, ay and (ax - ay) does not overflow or underflow.  */
static inline long double
kernel (long double ax, long double ay)
{
  long double t1, t2;
  long double h = sqrtl (ax * ax + ay * ay);
  if (h == 0.0)
    return h;
  if (h <= 2.0L * ay)
    {
      long double delta = h - ay;
      t1 = ax * (2.0L * delta - ax);
      t2 = (delta - 2.0L * (ax - ay)) * delta;
    }
  else
    {
      long double delta = h - ax;
      t1 = 2.0L * delta * (ax - 2.0L * ay);
      t2 = (4.0L * delta - ay) * ay + delta * delta;
    }

  h -= (t1 + t2) / (2.0L * h);
  return h;
}

long double
__ieee754_hypotl (long double x, long double y)
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

  long double ax = x < y ? y : x;
  long double ay = x < y ? x : y;

  /* If ax is huge, scale both inputs down.  */
  if (__glibc_unlikely (ax > SQRT_LDBL_MAX))
    {
      if (ay <= ax * SQRT_EPS_DIV_2)
	return (ay == 0.0) ? ax : ax + LDBL_TRUE_MIN;

      return kernel (ax * SCALE, ay * SCALE) / SCALE;
    }

  /* Common case: ax is not huge and ay is not tiny.  */
  if (__glibc_unlikely (ay < SQRT_LDBL_MIN))
    {
      /* Widely varying operands.  The LDBL_MIN_THRESHOLD check is used to avoid
	 a spurious underflow from the multiplication.  */
      if (__glibc_unlikely (ax >= LDBL_MIN_THRESHOLD && ay <= ax * SQRT_EPS_DIV_2))
	return (ay == 0.0) ? ax : ax + LDBL_TRUE_MIN;

      ax = math_narrow_eval (kernel (ax / SCALE, ay / SCALE) * SCALE);
      math_check_force_underflow_nonneg (ax);
      return ax;
    }

  /* Common case: ax is not huge and ay is not tiny.  */
  if (ay <= ax * SQRT_EPS_DIV_2)
    return (ay == 0.0) ? ax : ax + LDBL_TRUE_MIN;

  return kernel (ax, ay);
}
libm_alias_finite (__ieee754_hypotl, __hypotl)
