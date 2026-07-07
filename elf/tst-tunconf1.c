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
  size_t perturb = TUNABLE_GET_FULL (glibc, malloc, perturb, size_t, NULL);
  size_t mmap_threshold = TUNABLE_GET_FULL (glibc, malloc, mmap_threshold, size_t, NULL);
  size_t trim_threshold = TUNABLE_GET_FULL (glibc, malloc, trim_threshold, size_t, NULL);

  printf("tcache count is %ld (should be 5, from env)\n", (long)tcache_count);
  TEST_COMPARE ((long)tcache_count, 5);
  printf("tcache max is %ld (should be 4, from /etc)\n", (long)tcache_max);
  TEST_COMPARE ((long)tcache_max, 4);

  /* This is set by the environment but blocked by the config.  */
  printf("perturb is %ld (should be 42, from /etc)\n",
	 (long)perturb);
  TEST_COMPARE ((long)perturb, 42);

  /* This is blocked by the general config, enabled by filter, set in env.  */
  printf("mmap_threshold is %ld (should be 10002, from env)\n",
	 (long)mmap_threshold);
  TEST_COMPARE ((long)mmap_threshold, 10002);

  /* This is allowed by the general config, blocked by filter, set in env.  */
  printf("trim_threshold is %ld (should be 10001, from filter)\n",
	 (long)trim_threshold);
  TEST_COMPARE ((long)trim_threshold, 10001);

  return 0;
}

#include <support/test-driver.c>
