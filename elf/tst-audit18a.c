/* Check if DT_AUDIT a module without la_plt{enter,exit} symbols does not incur
   in profiling (BZ#15533).
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#include <link.h>
#include <stdio.h>

/* We interpose the profile resolver and if it is called it means profiling is
   enabled.  */
void
_dl_runtime_profile (ElfW(Word) addr)
{
  volatile int *p = NULL;
  *p = 0;
}

static int
do_test (void)
{
  printf ("...");
  return 0;
}

#include <support/test-driver.c>
