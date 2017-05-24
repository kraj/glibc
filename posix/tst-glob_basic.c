/* Basic glob tests.
   Copyright (C) 2001-2017 Free Software Foundation, Inc.
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

#include <glob.h>
#include <stdio.h>
#include <string.h>

#include <support/check.h>

static int
do_test (void)
{
  glob_t g;
  g.gl_pathc = 0;

  TEST_VERIFY_EXIT (glob ("", 0, NULL, &g) == GLOB_NOMATCH);
  TEST_VERIFY_EXIT (g.gl_pathc == 0);

  TEST_VERIFY_EXIT (glob ("", GLOB_NOCHECK, NULL, &g) == 0);
  TEST_VERIFY_EXIT (g.gl_pathc == 1);
  TEST_VERIFY_EXIT (strcmp (g.gl_pathv[0], "") == 0);

  return 0;
}

#include <support/test-driver.c>
