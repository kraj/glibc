/* Test the Mersenne Twister random functions.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#include <stdint.h>
#include <stdio.h>

#include <support/check.h>
#include <support/support_random.h>

static int
do_test (void)
{
  {
    struct mt19937_32 mt32;
    mt32_seed (&mt32, 0);
    for (int i = 0; i < 9999; ++i)
      mt32_rand (&mt32);
    TEST_VERIFY (mt32_rand (&mt32) == 4123659995ul);
  }

  {
    struct mt19937_64 mt64;
    mt64_seed (&mt64, 0);
    for (int i = 0; i < 9999; ++i)
      mt64_rand (&mt64);
    TEST_VERIFY (mt64_rand (&mt64) == UINT64_C(9981545732273789042));
  }

#define CHECK_UNIFORM_32(min, max)						\
  ({										\
    uint32_t v = uniform_uint32_distribution (mt32_rand (&mt32), min, max);	\
    TEST_VERIFY (v >= min && v <= max);						\
  })

  {
    struct mt19937_32 mt32;
    uint32_t seed;
    random_seed (&seed, sizeof (seed));
    mt32_seed (&mt32, seed);

    CHECK_UNIFORM_32 (0, 100);
    CHECK_UNIFORM_32 (100, 200);
    CHECK_UNIFORM_32 (100, 1<<10);
    CHECK_UNIFORM_32 (1<<10, UINT16_MAX);
    CHECK_UNIFORM_32 (UINT16_MAX, UINT32_MAX);
  }

#define CHECK_UNIFORM_64(min, max)						\
  ({										\
    uint64_t v = uniform_uint64_distribution (mt64_rand (&mt64), min, max);	\
    TEST_VERIFY (v >= min && v <= max);						\
  })

  {
    struct mt19937_64 mt64;
    uint64_t seed;
    random_seed (&seed, sizeof (seed));
    mt64_seed (&mt64, seed);

    CHECK_UNIFORM_64 (0, 100);
    CHECK_UNIFORM_64 (100, 200);
    CHECK_UNIFORM_64 (100, 1<<10);
    CHECK_UNIFORM_64 (1<<10, UINT16_MAX);
    CHECK_UNIFORM_64 (UINT16_MAX, UINT32_MAX);
    CHECK_UNIFORM_64 (UINT64_C(1)<<33, UINT64_C(1)<<34);
    CHECK_UNIFORM_64 (UINT64_C(1)<<34, UINT64_MAX);
  }

  return 0;
}

#include <support/test-driver.c>
