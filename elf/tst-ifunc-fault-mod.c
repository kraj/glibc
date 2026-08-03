/* Shared object exporting an IFUNC symbol with a resolver which crashes.
   Copyright (C) 2026 Free Software Foundation, Inc.
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

#include <stddef.h>

static void
implementation (void)
{
  /* Produce a crash, without depending on any relocations.  */
  volatile char *volatile p = NULL;
  *p = 0;
}

static __typeof__ (implementation) *
resolver (void)
{
  /* Produce a crash, without depending on any relocations.  */
  volatile char *volatile p = NULL;
  *p = 0;
  return implementation;
}

void magic (void) __attribute__ ((ifunc ("resolver")));
