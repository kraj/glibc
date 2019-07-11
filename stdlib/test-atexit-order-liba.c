/* Helper module for atexit execution order test.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int exit_code;

static void
liba_atexit (void)
{
  printf ("%s\n", __func__);
  fflush (stdout);
  _exit (exit_code);
}

static void
__attribute__((constructor))
liba_constructor (void)
{
  printf ("%s\n", __func__);
  atexit (liba_atexit);
}

void
set_a_exit_code (int code)
{
  exit_code = code;
}
