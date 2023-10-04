/* Check tunable parsing.
   Copyright (C) 2023 Free Software Foundation, Inc.
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

#include <array_length.h>
#include <dl-tunables.h>
#include <getopt.h>
#include <intprops.h>
#include <support/capture_subprocess.h>
#include <support/check.h>

static int restart;
#define CMDLINE_OPTIONS \
  { "restart", no_argument, &restart, 1 },

static const struct test_t
{
  const char *env;
  int32_t expected_malloc_check;
  int32_t expected_mmap_threshold;
  int32_t expected_perturb;
} tests[] =
{
  {
    "glibc.malloc.check=2:glibc.malloc.mmap_threshold=4096",
    2,
    4096,
    0,
  },
  /* Multiple values are parse from left to right.  */
  {
    "glibc.malloc.check=2:glibc.malloc.check=2:glibc.malloc.mmap_threshold=4096",
    2,
    4096,
    0,
  },
  {
    "glibc.malloc.check=2:glibc.malloc.mmap_threshold=4096:glibc.malloc.check=2",
    2,
    4096,
    0,
  },
  {
    ":glibc.malloc.garbage=2:glibc.malloc.check=1",
    1,
    0,
    0,
  },
  /* 0x800 is larger than tunable maxval (0xff).  */
  {
    "glibc.malloc.perturb=0x800",
    0,
    0,
    0,
  },
  {
    "glibc.malloc.perturb=0x55",
    0,
    0,
    0x55,
  },
  {
    "glibc.malloc.perturb=0x800:glibc.malloc.mmap_threshold=4096",
    0,
    4096,
    0,
  },
  /* Invalid keys are ignored.  */
  {
    "glibc.malloc.perturb=0x800:not_valid.malloc.check=2:glibc.malloc.mmap_threshold=4096",
    0,
    4096,
    0,
  },
  {
    "glibc.not_valid.check=2:glibc.malloc.mmap_threshold=4096",
    0,
    4096,
    0,
  },
  {
    "not_valid.malloc.check=2:glibc.malloc.mmap_threshold=4096",
    0,
    4096,
    0,
  },
  {
    "glibc.malloc.check=2",
    2,
    0,
    0,
  },
  /* Invalid subkeys are ignored.  */
  {
    "glibc.malloc.garbage=2:glibc.maoc.mmap_threshold=4096:glibc.malloc.check=2",
    2,
    0,
    0,
  },
  {
    "glibc.malloc.check=4:glibc.malloc.garbage=2:glibc.maoc.mmap_threshold=4096",
    0,
    0,
    0,
  },
  /* Tunable are processed from left to right, so last one is the one set.  */
  {
    "glibc.malloc.check=1:glibc.malloc.check=2",
    2,
    0,
    0,
  },
  {
    "not_valid.malloc.check=2",
    0,
    0,
    0,
  },
  {
    "glibc.not_valid.check=2",
    0,
    0,
    0,
  },
  /* Ill-formatted tunables are ignored.  */
  {
    "glibc.malloc.check=2=2",
    0,
    0,
    0,
  },
  {
    "glibc.malloc.mmap_threshold=glibc.malloc.mmap_threshold=4096",
    0,
    0,
    0,
  },
  /* If there is a ill-formatted key=value, everything after is also ignored.  */
  {
    "glibc.malloc.check=2=2:glibc.malloc.check=2",
    0,
    0,
    0,
  },
  /* Valid tunables set before ill-formatted ones are set.  */
  {
    "glibc.malloc.check=2:glibc.malloc.mmap_threshold=4096=4096",
    2,
    0,
    0,
  }
};

static int
handle_restart (int i)
{
  TEST_COMPARE (tests[i].expected_malloc_check,
		TUNABLE_GET_FULL (glibc, malloc, check, int32_t, NULL));
  TEST_COMPARE (tests[i].expected_mmap_threshold,
		TUNABLE_GET_FULL (glibc, malloc, mmap_threshold, int32_t, NULL));
  TEST_COMPARE (tests[i].expected_perturb,
		TUNABLE_GET_FULL (glibc, malloc, perturb, int32_t, NULL));
  return 0;
}

static int
do_test (int argc, char *argv[])
{
  /* We must have either:
     - One our fource parameters left if called initially:
       + path to ld.so         optional
       + "--library-path"      optional
       + the library path      optional
       + the application name
       + the test to check  */

  TEST_VERIFY_EXIT (argc == 2 || argc == 5);

  if (restart)
    return handle_restart (atoi (argv[1]));

  char nteststr[INT_BUFSIZE_BOUND (int)];

  char *spargv[10];
  int i = 0;
  for (; i < argc - 1; i++)
    spargv[i] = argv[i + 1];
  spargv[i++] = (char *) "--direct";
  spargv[i++] = (char *) "--restart";
  spargv[i++] = nteststr;
  spargv[i] = NULL;

  for (int i = 0; i < array_length (tests); i++)
    {
      snprintf (nteststr, sizeof nteststr, "%d", i);

      printf ("[%d] Spawned test for %s\n", i, tests[i].env);
      setenv ("GLIBC_TUNABLES", tests[i].env, 1);
      struct support_capture_subprocess result
	= support_capture_subprogram (spargv[0], spargv);
      support_capture_subprocess_check (&result, "tst-tunables", 0,
					sc_allow_stderr);
      support_capture_subprocess_free (&result);
    }

  return 0;
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
