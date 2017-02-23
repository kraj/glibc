/* Mask off bits.  Generic C version.
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

#ifndef STRING_MASKOFF_H
#define STRING_MASKOFF_H 1

#include <endian.h>
#include <stdint.h>
#include <string-optype.h>

/* Provide a mask based on the pointer alignment that sets up non-zero
   bytes before the beginning of the word.  It is used to mask off
   undesirable bits from an aligned read from an unaligned pointer.
   For instance, on a 64 bits machine with a pointer alignment of
   3 the function returns 0x0000000000ffffff for LE and 0xffffff0000000000
   (meaning to mask off the initial 3 bytes).  */
static inline op_t
create_mask (uintptr_t i)
{
  i = i % sizeof (op_t);
  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    return ~(((op_t)-1) << (i * CHAR_BIT));
  else
    return ~(((op_t)-1) >> (i * CHAR_BIT));
}

/* Setup an word with each byte being c_in.  For instance, on a 64 bits
   machine with input as 0xce the functions returns 0xcececececececece.  */
static inline op_t
repeat_bytes (unsigned char c_in)
{
  return ((op_t)-1 / 0xff) * c_in;
}

/* Based on mask created by 'create_mask', mask off the high bit of each
   byte in the mask.  It is used to mask off undesirable bits from an
   aligned read from an unaligned pointer, and also taking care to avoid
   match possible bytes meant to be matched.  For instance, on a 64 bits
   machine with a mask created from a pointer with an alignment of 3
   (0x0000000000ffffff) the function returns 0x7f7f7f0000000000 for BE
   and 0x00000000007f7f7f for LE.  */
static inline op_t
highbit_mask (op_t m)
{
  return m & repeat_bytes (0x7f);
}

/* Return the address of the op_t word containing the address P.  For
   instance on address 0x0011223344556677 and op_t with size of 8,
   it returns 0x0011223344556670.  */
static inline op_t *
word_containing (char const *p)
{
  return (op_t *) (p - (uintptr_t) p % sizeof (op_t));
}

#endif /* STRING_MASKOFF_H  */
