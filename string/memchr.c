/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <stdint.h>
#include <string-fza.h>
#include <string-fzb.h>
#include <string-fzi.h>
#include <string-maskoff.h>
#include <string-opthr.h>

#undef memchr

#ifndef MEMCHR
# define MEMCHR __memchr
#endif

/* Search no more than N bytes of S for C.  */
void *
MEMCHR (void const *s, int c_in, size_t n)
{
  const op_t *word_ptr, *lword;
  op_t repeated_c, before_mask, word;
  const char *lbyte;
  char *ret;
  uintptr_t s_int;

  if (__glibc_unlikely (n == 0))
    return NULL;

  s_int = (uintptr_t) s;

  /* Set up a word, each of whose bytes is C.  */
  repeated_c = repeat_bytes (c_in);
  before_mask = create_mask (s_int);

  /* Compute the address of the last byte taking in consideration possible
     overflow.  */
  uintptr_t lbyte_int = s_int + n - 1;
  lbyte_int |= -(lbyte_int < s_int);
  lbyte = (const char *) lbyte_int;

  /* Compute the address of the word containing the last byte. */
  lword = word_containing (lbyte);

  /* Read the first word, but munge it so that bytes before the array
     will not match goal.  */
  word_ptr = word_containing (s);
  word = (*word_ptr | before_mask) ^ (repeated_c & before_mask);

  while (has_eq (word, repeated_c) == 0)
    {
      if (word_ptr == lword)
	return NULL;
      word = *++word_ptr;
    }

  /* We found a match, but it might be in a byte past the end
     of the array.  */
  ret = (char *) word_ptr + index_first_eq (word, repeated_c);
  return (ret <= lbyte) ? ret : NULL;
}
weak_alias (__memchr, memchr)
libc_hidden_builtin_def (memchr)
