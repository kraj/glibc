/* Test log2p1.
   Copyright (C) 2024-2025 Free Software Foundation, Inc.
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

#include "libm-test-driver.c"

static const struct test_f_f_data log2p1_test_data[] =
  {
    TEST_f_f (log2p1, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (log2p1, -2, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (log2p1, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM|XFAIL_ROUNDING_IBM128_LIBGCC),
    TEST_f_f (log2p1, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),

    TEST_f_f (log2p1, plus_infty, plus_infty, ERRNO_UNCHANGED),
    TEST_f_f (log2p1, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (log2p1, -qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (log2p1, snan_value, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log2p1, -snan_value, qnan_value, INVALID_EXCEPTION),

    AUTO_TESTS_f_f (log2p1),
  };

static void
log2p1_test (void)
{
  ALL_RM_TEST (log2p1, 0, log2p1_test_data, RUN_TEST_LOOP_f_f, END);
}

static void
do_test (void)
{
  log2p1_test ();
}

/*
 * Local Variables:
 * mode:c
 * End:
 */
