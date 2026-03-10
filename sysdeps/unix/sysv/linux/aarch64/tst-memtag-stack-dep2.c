/* Tests for MEMTAG support.
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

#include <sys/auxv.h>
#include <support/capture_subprocess.h>
#include <support/check.h>

#include "tst-mte-helper.h"

static int
do_test (void)
{
  if (!(getauxval (AT_HWCAP2) & HWCAP2_MTE))
    FAIL_UNSUPPORTED ("kernel or CPU does not support or enable MTE");

  /* If any shared library requires MTE stack support, but the main binary has
     not DT_AARCH64_MEMTAG_MODE, defaults to asynchronous mode.  */
  TEST_VERIFY_EXIT (mte_enable ());
  TEST_VERIFY_EXIT (mte_mode () == PR_MTE_TCF_ASYNC);

  {
    struct support_capture_subprocess result =
      support_capture_subprocess (run_mte_test, (void*)TEST_MAIN);

    support_capture_subprocess_check (&result, "MTE main stack",
				      EXIT_MTESERR, sc_allow_none);

    support_capture_subprocess_free (&result);
  }

  {
    struct support_capture_subprocess result =
      support_capture_subprocess (run_mte_test, (void*)TEST_THREAD);

    support_capture_subprocess_check (&result, "MTE thread stack",
				      EXIT_MTESERR, sc_allow_none);

    support_capture_subprocess_free (&result);
  }

  return 0;
}

#include <support/test-driver.c>
