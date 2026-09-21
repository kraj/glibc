/* Correctly-rounded cubic root of binary64 value.

Copyright (c) 2021-2026 Alexei Sibidanov.

The original version of this file was copied from the CORE-MATH
project (file src/binary64/cbrt/cbrt.c, revision 2863c7f2).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <fenv.h>
#include <get-rounding-mode.h>
#include <libm-alias-double.h>
#include <math.h>
#include <stdint.h>
#include "math_config.h"

static inline unsigned int
cbrt_rounding_index (void)
{
  switch (get_rounding_mode ())
    {
    case FE_TONEAREST:
      return 0;
    case FE_DOWNWARD:
      return 1;
    case FE_UPWARD:
      return 2;
    case FE_TOWARDZERO:
      return 3;
    default:
      __builtin_unreachable ();
    }
}

double
__cbrt (double x)
{
  static const double escale[3] =
    {
      1.0,
      0x1.428a2f98d728bp+0, /* 2^(1/3) */
      0x1.965fea53d6e3dp+0, /* 2^(2/3) */
    };
  /* The polynomial c[0] + c[1]*x + c[2]*x^2 + c[3]*x^3 approximates x^(1/3)
     on [1,2] with maximal error < 9.2e-5 (attained at x=2).  */
  static const double c[] =
    {
      0x1.1b0babccfef9cp-1, 0x1.2c9a3e94d1da5p-1, -0x1.4dc30b1a1ddbap-3,
      0x1.7a8d3e4ec9b07p-6
    };
  const double u0 = 0x1.5555555555555p-2, u1 = 0x1.c71c71c71c71cp-3;
  static const double rsc[] = { 1, -1, 0.5, -0.5, 0.25, -0.25 };
  static const double off[] = { 0x1p-53, 0, 0, 0 };

  unsigned int rm = cbrt_rounding_index ();
  uint64_t hx = asuint64 (x);
  uint64_t mant = get_mantissa (hx);
  uint64_t sgn = hx & SIGN_MASK;
  unsigned int sign = hx >> (BIT_WIDTH - 1);
  unsigned int e = (hx >> MANTISSA_WIDTH) & 0x7ff;

  if (__glibc_unlikely (((e + 1) & 0x7ff) < 2))
    {
      uint64_t ix = hx & EXP_MANT_MASK;
      /* 0, inf, nan: return x + x instead of simply x, so that a signaling
	 NaN correctly triggers the invalid exception.  */
      if (e == 0x7ff || ix == 0)
	return x + x;
      /* Subnormal.  */
      int nz = stdc_leading_zeros (ix) - EXPONENT_WIDTH;
      mant <<= nz;
      mant &= MANTISSA_MASK;
      e -= nz - 1;
    }

  e += 3072;
  uint64_t one = (uint64_t) EXPONENT_BIAS << MANTISSA_WIDTH;
  unsigned int et = e / 3, it = e % 3;
  /* 2^(3k+it) <= x < 2^(3k+it+1), with 0 <= it <= 3.  */
  double zz = asdouble (((mant | one) + ((uint64_t) it << MANTISSA_WIDTH))
			| sgn);
  /* cbrt(x) = cbrt(zz) * 2^(et-1365) where 1 <= zz < 8.  */
  double isc = asdouble (asuint64 (escale[it]) | sgn);
  double z = asdouble (mant | one);
  /* cbrt(zz) = cbrt(z) * isc, where isc encodes 1, 2^(1/3) or 2^(2/3),
     and 1 <= z < 2.  */
  double r = 1 / z, rr = r * rsc[it << 1 | sign], z2 = z * z;
  double c0 = c[0] + z * c[1], c2 = c[2] + z * c[3];
  double y = c0 + z2 * c2, y2 = y * y;
  /* y is an approximation of z^(1/3).  */
  double h = y2 * (y * r) - 1;
  /* h determines the error between y and z^(1/3).  */
  y -= (h * y) * (u0 - u1 * h);
  /* The correction y -= (h*y)*(u0 - u1*h) corresponds to a cubic variant of
     Newton's method, with the function f(y) = 1-z/y^3.  */
  y *= isc;
  /* Now y is an approximation of zz^(1/3), and rr an approximation of 1/zz.
     We now perform another iteration of Newton-Raphson, this time with a
     linear approximation only.  */
  y2 = y * y;
  double y2l = fma (y, y, -y2);
  /* y2 + y2l = y^2 exactly.  */
  double y3 = y2 * y, y3l = fma (y, y2, -y3) + y * y2l;
  /* y3 + y3l approximates y^3 with about 106 bits of accuracy.  */
  h = ((y3 - zz) + y3l) * rr;
  double dy = h * (y * u0);
  /* The approximation of zz^(1/3) is y - dy.  */
  double y1 = y - dy;
  dy = (y - y1) - dy;
  /* The approximation of zz^(1/3) is now y1 + dy, where |dy| < 1/2 ulp(y)
     (for rounding to nearest).  */
  double ady = fabs (dy);
  /* For directed roundings, ady0 is tiny when dy is tiny, or ady0 is near
     ulp(1); for rounding to nearest, ady0 is tiny when dy is near 1/2 ulp(1),
     or 3/2 ulp(1).  */
  double ady0 = fabs (ady - off[rm]);
  double ady1 = fabs (ady - (0x1p-52 + off[rm]));
  if (__glibc_unlikely (ady0 < 0x1p-75 || ady1 < 0x1p-75))
    {
      y2 = y1 * y1;
      y2l = fma (y1, y1, -y2);
      y3 = y2 * y1;
      y3l = fma (y1, y2, -y3) + y1 * y2l;
      h = ((y3 - zz) + y3l) * rr;
      dy = h * (y1 * u0);
      y = y1 - dy;
      dy = (y1 - y) - dy;
      y1 = y;
      ady = fabs (dy);
      ady0 = fabs (ady - off[rm]);
      ady1 = fabs (ady - (0x1p-52 + off[rm]));
      if (__glibc_unlikely (ady0 < 0x1p-98 || ady1 < 0x1p-98))
	{
	  double azz = fabs (zz);
	  /* ~ 0x1.79d15d0e8d59b80000000000000ffc3dp+0 */
	  if (azz == 0x1.9b78223aa307cp+1)
	    y1 = copysign (0x1.79d15d0e8d59cp+0, zz);
	  /* ~ 0x1.de87aa837820e80000000000001c0f08p+0 */
	  if (azz == 0x1.a202bfc89ddffp+2)
	    y1 = copysign (0x1.de87aa837820fp+0, zz);
	  if (rm > 0)
	    {
	      static const double wlist[][2] =
		{
		  /* ~ 0x1.1236160ba9b930000000000001e7e8fap+0 */
		  { 0x1.3a9ccd7f022dbp+0, 0x1.1236160ba9b93p+0 },
		  /* ~ 0x1.23115e657e49c0000000000001d7a799p+0 */
		  { 0x1.7845d2faac6fep+0, 0x1.23115e657e49cp+0 },
		  /* ~ 0x1.388fb44cdcf5a0000000000002202c55p+0 */
		  { 0x1.d1ef81cbbbe71p+0, 0x1.388fb44cdcf5ap+0 },
		  /* ~ 0x1.46bcbf47dc1e8000000000000303aa2dp+0 */
		  { 0x1.0a2014f62987cp+1, 0x1.46bcbf47dc1e8p+0 },
		  /* ~ 0x1.95decfec9c9040000000000000159e8ep+0 */
		  { 0x1.fe18a044a5501p+1, 0x1.95decfec9c904p+0 },
		  /* ~ 0x1.e05335a6401de00000000000027ca017p+0 */
		  { 0x1.a6bb8c803147bp+2, 0x1.e05335a6401dep+0 },
		  /* ~ 0x1.e281d87098de80000000000000ee9314p+0 */
		  { 0x1.ac8538a031cbdp+2, 0x1.e281d87098de8p+0 },
		};
	      for (int i = 0; i < 7; i++)
		if (azz == wlist[i][0])
		  y1 = copysign (wlist[i][1]
				 + ((rm + sign == 2) ? 0x1p-52 : 0), zz);
	    }
	}
    }

  uint64_t res = asuint64 (y1)
		 + ((uint64_t) (et - 342 - EXPONENT_BIAS) << MANTISSA_WIDTH);
  int64_t m0 = res << 30, m1 = m0 >> (BIT_WIDTH - 1);
  if (__glibc_unlikely ((uint64_t) (m0 ^ m1) <= (UINT64_C (1) << 30)))
    {
      double t = asdouble ((asuint64 (y1) + (UINT64_C (1) << 15))
			   & UINT64_C (0xffffffffffff0000));
      if (fabs ((t - y1) - dy) < 0x1p-60 || fabs (zz) == 1.0)
	res = (res + (UINT64_C (1) << 15)) & UINT64_C (0xffffffffffff0000);
    }
  return asdouble (res);
}
libm_alias_double (__cbrt, cbrt)
