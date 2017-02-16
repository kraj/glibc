/* string-fzi.h -- zero byte detection; indices.  Alpha version.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#ifndef STRING_FZI_H
#define STRING_FZI_H

#include <limits.h>
#include <string-optype.h>

/* Note that since CMPBGE creates a bit mask rather than a byte mask,
   we cannot simply provide a target-specific string-fza.h.  */

/* A subroutine for the index_zero functions.  Given a bitmask C,
   return the index of the first bit set in memory order.  */

static inline unsigned int
index_first_ (unsigned long int c)
{
#ifdef __alpha_cix__
  return __builtin_ctzl (c);
#else
  c = c & -c;
  return (c & 0xf0 ? 4 : 0) + (c & 0xcc ? 2 : 0) + (c & 0xaa ? 1 : 0);
#endif
}

/* Similarly, but return the (memory order) index of the last bit
   that is non-zero.  Note that only the least 8 bits may be nonzero.  */

static inline unsigned int
index_last_ (unsigned long int x)
{
#ifdef __alpha_cix__
  return __builtin_clzl (x) ^ 63;
#else
  unsigned r = 0;
  if (x & 0xf0)
    r += 4;
  if (x & (0xc << r))
    r += 2;
  if (x & (0x2 << r))
    r += 1;
  return r;
#endif
}

/* Given a word X that is known to contain a zero byte, return the
   index of the first such within the word in memory order.  */

static inline unsigned int
index_first_zero (op_t x)
{
  return index_first_ (__builtin_alpha_cmpbge (0, x));
}

/* Similarly, but perform the test for byte equality between X1 and X2.  */

static inline unsigned int
index_first_eq (op_t x1, op_t x2)
{
  return index_first_zero (x1 ^ x2);
}

/* Similarly, but perform the search for zero within X1 or
   equality between X1 and X2.  */

static inline unsigned int
index_first_zero_eq (op_t x1, op_t x2)
{
  return index_first_ (__builtin_alpha_cmpbge (0, x1)
		       | __builtin_alpha_cmpbge (0, x1 ^ x2));
}

/* Similarly, but perform the search for zero within X1 or
   inequality between X1 and X2.  */

static inline unsigned int
index_first_zero_ne (op_t x1, op_t x2)
{
  return index_first_ (__builtin_alpha_cmpbge (0, x1)
		       | (__builtin_alpha_cmpbge (0, x1 ^ x2) ^ 0xFF));
}

/* Similarly, but search for the last zero within X.  */

static inline unsigned int
index_last_zero (op_t x)
{
  return index_last_ (__builtin_alpha_cmpbge (0, x));
}

static inline unsigned int
index_last_eq (op_t x1, op_t x2)
{
  return index_last_zero (x1 ^ x2);
}

#endif /* STRING_FZI_H */
