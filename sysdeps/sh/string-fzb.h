/* Zero byte detection; boolean.  SH4 version.
   Copyright (C) 2018 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#ifndef STRING_FZB_H
#define STRING_FZB_H 1

#include <string-optype.h>

/* Determine if any byte within X is zero.  This is a pure boolean test.  */

static inline _Bool
has_zero (op_t x)
{
  op_t zero = 0x0, ret;
  asm volatile ("cmp/str %1,%2\n"
		"movt %0\n"
		: "=r" (ret)
		: "r" (zero), "r" (x));
  return ret;
}

/* Likewise, but for byte equality between X1 and X2.  */

static inline _Bool
has_eq (op_t x1, op_t x2)
{
  return has_zero (x1 ^ x2);
}

/* Likewise, but for zeros in X1 and equal bytes between X1 and X2.  */

static inline _Bool
has_zero_eq (op_t x1, op_t x2)
{
  return has_zero (x1) | has_eq (x1, x2);
}

#endif /* STRING_FZB_H */
