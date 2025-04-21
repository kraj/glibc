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
#include <libm-alias-float.h>
#include <math-type-macros-float.h>
#include "math_config.h"

#ifdef DEF_AS_LLOGBF
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
M_DECL_FUNC (IMPL_NAME) (float x)
{
  uint32_t ux = asuint (x);
  int ex = (ux & ~SIGN_MASK) >> MANTISSA_WIDTH;
  if (ex == 0) /* zero or subnormal */
    {
      /* Clear sign and exponent.  */
      ux <<= 1 + EXPONENT_WIDTH;
      if (ux == 0)
	return invalid_ret (RET_LOGB0);
      /* sbunormal */
      return (RET_TYPE)-127 - stdc_leading_zeros (ux);
    }
  if (ex == EXPONENT_MASK >> MANTISSA_WIDTH) /* NaN or Inf */
    return invalid_ret (ux << (1 + EXPONENT_WIDTH) ? RET_LOGBNAN : RET_LOGMAX);
  return ex - 127;
}
libm_alias_float (IMPL_NAME, FUNC_NAME);
