/* Test that the tunables cache can override env vars.
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
#include <support/check.h>

#include "dl-tunables.h"

static int
do_test (void)
{
  size_t tcache_count = TUNABLE_GET_FULL (glibc, malloc, tcache_count, size_t, NULL);
  size_t tcache_max = TUNABLE_GET_FULL (glibc, malloc, tcache_max, size_t, NULL);
  printf("tcache count is %ld (should be 5, from env)\n", (long)tcache_count);
  TEST_COMPARE ((long)tcache_count, 5);
  printf("tcache max is %ld (should be 4, from /etc)\n", (long)tcache_max);
  TEST_COMPARE ((long)tcache_max, 4);
  return 0;
}

#include <support/test-driver.c>
