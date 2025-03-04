/* Test matherr (compat symbols, binary modifies library's _LIB_VERSION).
   Copyright (C) 1997-2025 Free Software Foundation, Inc.
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <math-svid-compat.h>
#include <shlib-compat.h>

#undef matherr
#undef _LIB_VERSION
compat_symbol_reference (libm, matherr, matherr, GLIBC_2_0);
compat_symbol_reference (libm, _LIB_VERSION, _LIB_VERSION, GLIBC_2_0);

static int fail = 1;

int
matherr (struct exception *s)
{
  printf ("matherr is working\n");
  fail = 0;
  return 1;
}

static int
do_test (void)
{
  _LIB_VERSION = _SVID_;
  acos (2.0);
  return fail;
}

#include <support/test-driver.c>
