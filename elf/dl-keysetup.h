/* Compute secret keys used for protection heuristics.
   Copyright (C) 2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef KEY_SETUP_H
#define KEY_SETUP_H

#include <stdint.h>

/* The set of protection keys used by glibc.  */
struct key_setup
{
  /* Canary for the stack-smashing protector.  */
  uintptr_t stack;

  /* Pointer guard, protecting selected function pointers.  */
  uintptr_t pointer;

  /* Heap guard, protecting the malloc chunk header.  */
  uintptr_t heap_header;

  /* Heap guard part two, protecting the previous chunk size field.  */
  uintptr_t heap_footer;
};

/* Derive the keys in *RESULT from RANDOM, which comes from the
   auxiliary vector and points to 16 bytes of randomness.  */
void __compute_keys (const void *random, struct key_setup *result)
  attribute_hidden;

#endif /* KEY_SETUP_H */
