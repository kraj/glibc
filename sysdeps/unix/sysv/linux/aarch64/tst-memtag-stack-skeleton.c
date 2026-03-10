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

#include <string.h>
#include <sys/auxv.h>
#include <support/capture_subprocess.h>
#include <support/check.h>

#include "tst-memtag-test-skeleton.c"

static int
do_test (int argc, char *argv[])
{
  if (!(getauxval (AT_HWCAP2) & HWCAP2_MTE))
    FAIL_UNSUPPORTED ("kernel or CPU does not support or enable MTE");

  TEST_VERIFY_EXIT (mte_enable ());
   /* We must have
     - one or four parameters left if called initially
       + path for ld.so         optional
       + "--library-path"       optional
       + the library path       optional
       + the application name
       + the expected MTE mode
  */
  TEST_VERIFY_EXIT (argc == 2);
  int mode = mte_mode ();
  if (strcmp (argv[1], "sync") == 0)
    TEST_VERIFY_EXIT (mode == PR_MTE_TCF_SYNC);
  else if (strcmp (argv[1], "async") == 0)
    TEST_VERIFY_EXIT (mode == PR_MTE_TCF_ASYNC);
  /* glibc.mem.tagging=0x3 ("auto") can e either sync, async, or asymm; so
     there it no point of checking it.  */

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
#define TEST_FUNCTION_ARGV do_test

#include <support/test-driver.c>
