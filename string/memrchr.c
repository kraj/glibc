/* memrchr -- find the last occurrence of a byte in a memory block
   Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Based on strlen implementation by Torbjorn Granlund (tege@sics.se),
   with help from Dan Sahlin (dan@sics.se) and
   commentary by Jim Blandy (jimb@ai.mit.edu);
   adaptation to memchr suggested by Dick Karpinski (dick@cca.ucsf.edu),
   and implemented by Roland McGrath (roland@ai.mit.edu).

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

#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <string-fzb.h>
#include <string-fzi.h>
#include <string-opthr.h>
#include <string-maskoff.h>

#undef __memrchr
#undef memrchr

#ifndef MEMRCHR
# define MEMRCHR __memrchr
#endif

void *
MEMRCHR (const void *s, int c_in, size_t n)
{
  uintptr_t s_int = (uintptr_t) s;
  uintptr_t lbyte_int = s_int + n;

  /* Handle the last few characters by reading one character at a time.
     Do this until CHAR_PTR is aligned on a word boundary, or
     the entirety of small inputs.  */
  const unsigned char *char_ptr = (const unsigned char *) lbyte_int;
  size_t align = lbyte_int % sizeof (op_t);
  if (n < OP_T_THRES || align > n)
    align = n;
  for (size_t i = 0; i < align; ++i)
    if (*--char_ptr == c_in)
      return (void *) char_ptr;

  const op_t *word_ptr = (const op_t *) char_ptr;
  n -= align;
  if (__glibc_unlikely (n == 0))
    return NULL;

  /* Compute the address of the word containing the initial byte. */
  const op_t *lword = word_containing (s);

  /* Set up a word, each of whose bytes is C.  */
  op_t repeated_c = repeat_bytes (c_in);

  char *ret;
  op_t word;

  while (word_ptr != lword)
    {
      word = *--word_ptr;
      if (has_eq (word, repeated_c))
	goto found;
    }
  return NULL;

found:
  /* We found a match, but it might be in a byte past the start 
     of the array.  */
  ret = (char *) word_ptr + index_last_eq (word, repeated_c);
  return (ret >= (char*) s) ? ret : NULL;
}
weak_alias (__memrchr, memrchr)
