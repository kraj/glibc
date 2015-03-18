/* Single-precision floating point square root.
   Copyright (C) 2010 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <math.h>
#include <math_private.h>
#include <fenv_libc.h>
#include <inttypes.h>

#include <sysdep.h>
#include <ldsodefs.h>

static const ieee_float_shape_type a_nan = {.word = 0x7fc00000 };
static const ieee_float_shape_type a_inf = {.word = 0x7f800000 };
static const float threehalf = 1.5;

/* The method is based on the descriptions in:

   _The Handbook of Floating-Pointer Arithmetic_ by Muller et al., chapter 5;
   _IA-64 and Elementary Functions: Speed and Precision_ by Markstein, chapter 9

   We find the reciprocal square root and use that to compute the actual
   square root.  */

#ifdef __STDC__
float
__ieee754_sqrtf (float b)
#else
float
__ieee754_sqrtf (b)
     float b;
#endif
{
  if (__builtin_expect (b > 0, 1))
    {
#define FMSUB(a_, c_, b_)                                               \
      ({ double __r;                                                    \
        __asm__ ("fmsub %[r], %[a], %[c], %[b]\n"                       \
                 : [r] "=f" (__r) : [a] "f" (a_), [c] "f" (c_), [b] "f" (b_)); \
        __r;})
#define FNMSUB(a_, c_, b_)                                              \
      ({ double __r;                                                    \
        __asm__ ("fnmsub %[r], %[a], %[c], %[b]\n"                      \
                 : [r] "=f" (__r) : [a] "f" (a_), [c] "f" (c_), [b] "f" (b_)); \
        __r;})

      if (__builtin_expect (b != a_inf.value, 1))
        {
          double y, x;
          fenv_t fe;

          fe = fegetenv_register ();

          relax_fenv_state ();

          /* Compute y = 1.5 * b - b.  Uses fewer constants than y = 0.5 * b.  */
          y = FMSUB (threehalf, b, b);

          /* Initial estimate.  */
          __asm__ ("frsqrte %[x], %[b]\n" : [x] "=f" (x) : [b] "f" (b));

          /* Iterate.  x_{n+1} = x_n * (1.5 - y * (x_n * x_n)).  */
          x = x * FNMSUB (y, x * x, threehalf);
          x = x * FNMSUB (y, x * x, threehalf);
          x = x * FNMSUB (y, x * x, threehalf);

          /* All done.  */
          fesetenv_register (fe);
          return x * b;
        }
    }
  else if (b < 0)
    {
      /* For some reason, some PowerPC32 processors don't implement
         FE_INVALID_SQRT.  */
#ifdef FE_INVALID_SQRT
      feraiseexcept (FE_INVALID_SQRT);

      fenv_union_t u = { .fenv = fegetenv_register () };
      if ((u.l & FE_INVALID) == 0)
#endif
	feraiseexcept (FE_INVALID);
      b = a_nan.value;
    }
  return f_washf (b);
}
strong_alias (__ieee754_sqrtf, __sqrtf_finite)
