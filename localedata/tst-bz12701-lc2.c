/* Verify scanf memory handling with the 'c' conversion (BZ #12701).
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

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <libc-diag.h>
#include <support/check.h>
#include <support/next_to_fault.h>
#include <support/xstdio.h>

static int
do_test (void)
{
  wchar_t *c = NULL;
  int i;

  TEST_VERIFY (sscanf ("1234", "%30mlc", &c) == 1);

  TEST_VERIFY (c != NULL);
  TEST_COMPARE_BLOB (c, 5 * sizeof (wchar_t),
		     L"1234\0", 5 * sizeof (wchar_t));
  for (i = 5; i < 30; i ++)
    TEST_VERIFY (c[i] == L'\0');

  TEST_VERIFY (malloc_usable_size (c) >= 30 * sizeof(wchar_t));

  return 0;
}

#include <support/test-driver.c>
