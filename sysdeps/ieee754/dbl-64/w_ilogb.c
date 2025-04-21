/* Get integer exponent of a floating-point value.
   Copyright (C) 1999-2025 Free Software Foundation, Inc.
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

#include <fenv.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdbit.h>
#include <libm-alias-double.h>
#include "math_config.h"

#ifdef DEF_AS_LLOGB
# define IMPL_NAME   __llogb
# define FUNC_NAME   llogb
# define RET_TYPE    long int
# define RET_LOGB0   FP_LLOGB0
# define RET_LOGBNAN FP_LLOGBNAN
# define RET_LOGMAX  LONG_MAX
#else
# define IMPL_NAME   __ilogb
# define FUNC_NAME   ilogb
# define RET_TYPE    int
# define RET_LOGB0   FP_ILOGB0
# define RET_LOGBNAN FP_ILOGBNAN
# define RET_LOGMAX  INT_MAX
#endif

static inline RET_TYPE
invalid_ret (RET_TYPE r)
{
  __set_errno (EDOM);
  __feraiseexcept (FE_INVALID);
  return r;
}

RET_TYPE
IMPL_NAME (double x)
{
  uint64_t ux = asuint64 (x);
  int ex = (ux & ~SIGN_MASK) >> MANTISSA_WIDTH;
  if (ex == 0) /* zero or subnormal */
    {
      /* Clear sign and exponent */
      ux <<= 12;
      if (ux == 0)
	return invalid_ret (RET_LOGB0);
      /* subnormal  */
      return (RET_TYPE)-1023 - stdc_leading_zeros (ux);
    }
  if (ex == EXPONENT_MASK >> MANTISSA_WIDTH) /* NaN or Inf */
    return invalid_ret (ux << 12 ? RET_LOGBNAN : RET_LOGMAX);
  return ex - 1023;
}
libm_alias_double (IMPL_NAME, FUNC_NAME)
