/* Test nextup.
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

#include "libm-test-driver.c"

static const struct test_f_f_data nextup_test_data[] =
  {
    TEST_f_f (nextup, minus_zero, min_subnorm_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, 0, min_subnorm_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, -min_subnorm_value, minus_zero, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, max_value, plus_infty, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, plus_infty, plus_infty, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, minus_infty, -max_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, -qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, snan_value, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION),
    TEST_f_f (nextup, -snan_value, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION),
#if TEST_COND_binary32
    TEST_f_f (nextup, 1.0, 0x1.000002p0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, -1.0, -0x0.ffffffp0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
#endif
#if TEST_COND_binary64
    TEST_f_f (nextup, 1.0, 0x1.0000000000001p+0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, -1.0, -0x1.fffffffffffffp-1, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
#endif
#if TEST_COND_m68k96 || TEST_COND_intel96
    TEST_f_f (nextup, 1.0L, 0x8.0000000000000010p-3L, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, -1.0L, -0xf.fffffffffffffff0p-4L, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, 0xf.fffffffffffffff0p-4L, 1.0L, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
#endif
#if TEST_COND_intel96
    TEST_f_f (nextup, -0x0.fffffffep-16382L, -0x0.fffffffdfffffffep-16382L, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
#endif
#if TEST_COND_m68k96
    TEST_f_f (nextup, -0x0.fffffffep-16383L, -0x0.fffffffdfffffffep-16383L, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
#endif
#if TEST_COND_ibm128
    TEST_f_f (nextup, 1.0L, 1.0L + 0x1p-105L, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, -1.0L - 0x1p-105L, -1.0L, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, -1.0L, -1.0L + 0x1p-106L, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
#endif
#if TEST_COND_binary128
    TEST_f_f (nextup, 1.0L, 0x1.0000000000000000000000000001p0L, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_f (nextup, -1.0L, -0x1.ffffffffffffffffffffffffffffp-1L, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
#endif
  };

static void
nextup_test (void)
{
  ALL_RM_TEST (nextup, 1, nextup_test_data, RUN_TEST_LOOP_f_f, END);
}

static void
do_test (void)
{
  nextup_test ();
}

/*
 * Local Variables:
 * mode:c
 * End:
 */
