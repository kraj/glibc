/* Test that glibc.cpu.mtemode selects corect mode for MTE tag checking.
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
#include <unistd.h>
#include <sys/auxv.h>

#include <array_length.h>
#include <support/check.h>
#include <support/xdlfcn.h>
#include <support/support.h>
#include <support/capture_subprocess.h>

#include <cpu-features.h>
#define SHARED
#include <ldsodefs.h>

static int
work (unsigned int expected)
{
  /* Avoid introducing a copy relocation due to the hidden alias in
   ld.so.  */
  struct rtld_global *gl = xdlsym (NULL, "_rtld_global");
  TEST_COMPARE (gl->_dl_aarch64_mte_mode, expected);
  return 0;
}

static int
do_test_argv (int argc, char *argv[])
{
  if (argc == 2)
    return work (atol (argv[1]));

  unsigned long hwcap2 = getauxval (AT_HWCAP2);
  if ((hwcap2 & HWCAP2_MTE) == 0)
    FAIL_UNSUPPORTED ("MTE is not supported by this system");

  struct test_entry
  {
    unsigned int mode;
    const char *tunable;
  };

  struct test_entry tests[] = {
    /* Test each explicit tunable value.  */
    { .mode = MTE_MODE_DISABLED, .tunable = "glibc.cpu.mtemode=disabled" },
    { .mode = MTE_MODE_ENABLED, .tunable = "glibc.cpu.mtemode=enabled" },
    { .mode = MTE_MODE_SYNC, .tunable = "glibc.cpu.mtemode=sync" },
    { .mode = MTE_MODE_ASYNC, .tunable = "glibc.cpu.mtemode=async" },
    /* Test wrong value and no tunable set.  */
    { .mode = MTE_MODE_ENABLED, .tunable = "glibc.cpu.mtemode=gibberish" },
    { .mode = MTE_MODE_DISABLED, .tunable = "" },
  };

  array_foreach (t, tests)
    {
      char *mode = xasprintf ("%u", t->mode);
      char *tunable = xasprintf ("GLIBC_TUNABLES=%s", t->tunable);
      char *subproc = xasprintf ("tst-mtemode-tunable-%s", t->tunable);
      char *spargv[] = { argv[0], mode, NULL, };
      char *spenvp[] = { tunable, NULL, };
      struct support_capture_subprocess result;
      result = support_capture_subprogram (spargv[0], spargv, spenvp);
      support_capture_subprocess_check (&result, subproc, 0, sc_allow_none);
      support_capture_subprocess_free (&result);
      free (mode);
      free (tunable);
      free (subproc);
    }

  return 0;
}

#define TEST_FUNCTION_ARGV do_test_argv
#include <support/test-driver.c>
