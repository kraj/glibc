/* Zero byte detection; indexes.  Generic C version.
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

#ifndef STRING_FZI_H
#define STRING_FZI_H 1

#include <limits.h>
#include <endian.h>
#include <string-fza.h>

/* An improved bitscan routine, multiplying the De Bruijn sequence with a
   0-1 mask separated by the least significant one bit of a scanned integer
   or bitboard [1].

   [1] https://chessprogramming.wikispaces.com/Kim+Walisch  */

static inline unsigned int
index_access (const op_t i)
{
  static const char index[] =
  {
# if __WORDSIZE == 64
     0, 47,  1, 56, 48, 27,  2, 60,
    57, 49, 41, 37, 28, 16,  3, 61,
    54, 58, 35, 52, 50, 42, 21, 44,
    38, 32, 29, 23, 17, 11,  4, 62,
    46, 55, 26, 59, 40, 36, 15, 53,
    34, 51, 20, 43, 31, 22, 10, 45,
    25, 39, 14, 33, 19, 30,  9, 24,
    13, 18,  8, 12,  7,  6,  5, 63
# else
     0,  9,  1, 10, 13, 21,  2, 29,
    11, 14, 16, 18, 22, 25,  3, 30,
     8, 12, 20, 28, 15, 17, 24,  7,
    19, 27, 23,  6, 26,  5,  4, 31
# endif
  };
  return index[i];
}

/* For architectures which only provides __builtin_clz{l} (HAVE_BUILTIN_CLZ)
   and/or __builtin_ctz{l} (HAVE_BUILTIN_CTZ) which uses external libcalls
   (for intance __c{l,t}z{s,d}i2 from libgcc) the following wrapper provides
   inline implementation for both count leading zeros and count trailing
   zeros using branchless computation.  As for builtin, if x is 0 the
   result is undefined.*/

static inline unsigned int
__ctz (op_t x)
{
#if !HAVE_BUILTIN_CTZ
  op_t i;
# if __WORDSIZE == 64
  i = (x ^ (x - 1)) * 0x03F79D71B4CB0A89ull >> 58;
# else
  i = (x ^ (x - 1)) * 0x07C4ACDDU >> 27;
# endif
  return index_access (i);
#else
  if (sizeof (op_t) == sizeof (long))
    return __builtin_ctzl (x);
  else
    return __builtin_ctzll (x);
#endif
};

static inline unsigned int
__clz (op_t x)
{
#if !HAVE_BUILTIN_CLZ
  unsigned r;
  op_t i;

  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
# if __WORDSIZE == 64
  x |= x >> 32;
  i = x * 0x03F79D71B4CB0A89ull >> 58;
# else
  i = x * 0x07C4ACDDU >> 27;
# endif
  r = index_access (i);
  return r ^ (sizeof (op_t) * CHAR_BIT - 1);
#else
  if (sizeof (op_t) == sizeof (long))
    return __builtin_clzl (x);
  else
    return __builtin_clzll (x);
#endif
}

/* A subroutine for the index_zero functions.  Given a test word C, return
   the (memory order) index of the first byte (in memory order) that is
   non-zero.  */

static inline unsigned int
index_first_ (op_t c)
{
  _Static_assert (sizeof (op_t) == sizeof (long)
		  || sizeof (op_t) == sizeof (long long),
		  "Unhandled word size");

  unsigned r;
  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    r = __ctz (c);
  else
    r = __clz (c);
  return r / CHAR_BIT;
}

/* Similarly, but return the (memory order) index of the last byte
   that is non-zero.  */

static inline unsigned int
index_last_ (op_t c)
{
  _Static_assert (sizeof (op_t) == sizeof (long)
		  || sizeof (op_t) == sizeof (long long),
		  "Unhandled word size");

  unsigned r;
  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    r = __clz (c);
  else
    r = __ctz (c);
  return sizeof (op_t) - 1 - (r / CHAR_BIT);
}

/* Given a word X that is known to contain a zero byte, return the
   index of the first such within the word in memory order.  */

static inline unsigned int
index_first_zero (op_t x)
{
  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    x = find_zero_low (x);
  else
    x = find_zero_all (x);
  return index_first_ (x);
}

/* Similarly, but perform the search for byte equality between X1 and X2.  */

static inline unsigned int
index_first_eq (op_t x1, op_t x2)
{
  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    x1 = find_eq_low (x1, x2);
  else
    x1 = find_eq_all (x1, x2);
  return index_first_ (x1);
}

/* Similarly, but perform the search for zero within X1 or
   equality between X1 and X2.  */

static inline unsigned int
index_first_zero_eq (op_t x1, op_t x2)
{
  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    x1 = find_zero_eq_low (x1, x2);
  else
    x1 = find_zero_eq_all (x1, x2);
  return index_first_ (x1);
}

/* Similarly, but perform the search for zero within X1 or
   inequality between X1 and X2.  */

static inline unsigned int
index_first_zero_ne (op_t x1, op_t x2)
{
  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    x1 = find_zero_ne_low (x1, x2);
  else
    x1 = find_zero_ne_all (x1, x2);
  return index_first_ (x1);
}

/* Similarly, but search for the last zero within X.  */

static inline unsigned int
index_last_zero (op_t x)
{
  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    x = find_zero_all (x);
  else
    x = find_zero_low (x);
  return index_last_ (x);
}

static inline unsigned int
index_last_eq (op_t x1, op_t x2)
{
  return index_last_zero (x1 ^ x2);
}

#endif /* STRING_FZI_H */
