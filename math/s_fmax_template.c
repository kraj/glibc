/* Return maximum numeric value of X and Y.
   Copyright (C) 1997-2021 Free Software Foundation, Inc.
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
#include <math-use-builtins.h>

#if __HAVE_FLOAT128
# define USE_BUILTIN_F128 , _Float128 : USE_FMAXF128_BUILTIN
# define BUILTIN_F128     , _Float128 :__builtin_fmaxf128
#else
# define USE_BUILTIN_F128
# define BUILTIN_F128
#endif

#define USE_BUILTIN(X, Y)                   \
  _Generic((X),                             \
	   float       : USE_FMAXF_BUILTIN, \
	   double      : USE_FMAX_BUILTIN,  \
	   long double : USE_FMAXL_BUILTIN  \
	   USE_BUILTIN_F128)

#define BUILTIN(X, Y)                       \
  _Generic((X),                             \
	   float       : __builtin_fmaxf,   \
	   double      : __builtin_fmax,    \
	   long double : __builtin_fmaxl    \
	   BUILTIN_F128)                    \
  (X, Y)

FLOAT
M_DECL_FUNC (__fmax) (FLOAT x, FLOAT y)
{
  if (USE_BUILTIN (x, y))
    return BUILTIN (x, y);

  if (isgreaterequal (x, y))
    return x;
  else if (isless (x, y))
    return y;
  else if (issignaling (x) || issignaling (y))
    return x + y;
  else
    return isnan (y) ? x : y;
}

declare_mgen_alias (__fmax, fmax);
