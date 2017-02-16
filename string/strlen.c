/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Torbjorn Granlund (tege@sics.se),
   with help from Dan Sahlin (dan@sics.se);
   commentary by Jim Blandy (jimb@ai.mit.edu).

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

#undef strlen

#ifndef STRLEN
# define STRLEN strlen
#endif

/* Return the length of the null-terminated string STR.  Scan for
   the null terminator quickly by testing four bytes at a time.  */
size_t
STRLEN (const char *str)
{
  /* Align pointer to sizeof op_t.  */
  const uintptr_t s_int = (uintptr_t) str;
  const op_t *word_ptr = word_containing (str);

  /* Read and MASK the first word. */
  op_t word = *word_ptr | create_mask (s_int);

  while (! has_zero (word))
    word = *++word_ptr;

  return ((const char *) word_ptr) + index_first_zero (word) - str;
}
libc_hidden_builtin_def (strlen)
