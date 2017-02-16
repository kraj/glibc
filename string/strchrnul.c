/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Based on strlen implementation by Torbjorn Granlund (tege@sics.se),
   with help from Dan Sahlin (dan@sics.se) and
   bug fix and commentary by Jim Blandy (jimb@ai.mit.edu);
   adaptation to strchr suggested by Dick Karpinski (dick@cca.ucsf.edu),
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
#include <stdlib.h>
#include <stdint.h>
#include <string-fza.h>
#include <string-fzb.h>
#include <string-fzi.h>
#include <string-maskoff.h>

#undef __strchrnul
#undef strchrnul

#ifndef STRCHRNUL
# define STRCHRNUL __strchrnul
#endif

/* Find the first occurrence of C in S or the final NUL byte.  */
char *
STRCHRNUL (const char *str, int c_in)
{
  const op_t *word_ptr;
  op_t found, word;

  /* Set up a word, each of whose bytes is C.  */
  op_t repeated_c = repeat_bytes (c_in);

  /* Align the input address to op_t.  */
  uintptr_t s_int = (uintptr_t) str;
  word_ptr = word_containing (str);

  /* Read the first aligned word, but force bytes before the string to
     match neither zero nor goal (we make sure the high bit of each byte
     is 1, and the low 7 bits are all the opposite of the goal byte).  */
  op_t bmask = create_mask (s_int);
  word = (*word_ptr | bmask) ^ (repeated_c & highbit_mask (bmask));

  while (! has_zero_eq (word, repeated_c))
    word = *++word_ptr;

  found = index_first_zero_eq (word, repeated_c);

  return (char *) (word_ptr) + found;
}

weak_alias (__strchrnul, strchrnul)
