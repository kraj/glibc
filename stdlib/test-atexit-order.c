/* Check atexit execution order regarding other exit types.
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

#include <string.h>
#include <stdlib.h>
#include <libgen.h>

#include <support/capture_subprocess.h>
#include <support/support.h>
#include <support/check.h>

static int
do_test (int argc, char *argv[])
{
  const int expected_exit_code = 32;
  char *expected_exit_code_str;
  char *test_path;

  /* We must have
     - one or four parameters left if called initially
       + argv[1]:   path for ld.so        optional
       + argv[2]:   "--library-path"      optional
       + argv[3]:   the library path      optional
       + argv[4/1]: the application name
   */
  char *spargv[6];
  {
    int i;
    for (i = 0; i < argc - 2; i++)
      spargv[i] = argv[i + 1];

    test_path = xasprintf ("%s/%s", dirname (argv[i + 1]),
			   "test-atexit-order-dabc");
    spargv[i++] = test_path;

    expected_exit_code_str = xasprintf ("%d", expected_exit_code);
    spargv[i++] = expected_exit_code_str;

    spargv[i] = NULL;
  }

  struct support_capture_subprocess result =
    support_capture_subprogram (spargv[0], spargv);

  const char expected[] = 
    "liba_constructor\n"
    "libb_constructor\n"
    "init_b\n"
    "libb_atexit\n"
    "liba_atexit\n";

  support_capture_subprocess_check (&result, "test-atexit-order",
                                    32, sc_allow_stdout);

  TEST_COMPARE_BLOB (result.out.buffer, result.out.length,
		     expected, sizeof (expected) - 1);

  support_capture_subprocess_free (&result);

  free (expected_exit_code_str);
  free (test_path);

  return 0;
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
