/* Verify that string tunables are sealed after early startup.
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

/* This generic test is parameterized by TST_SEAL_TUNABLE_NAME, the name of
   a glibc.cpu string tunable for the target ABI.  */

#include <stdlib.h>
#include <support/check.h>

#ifdef TST_SEAL_TUNABLE_NAME

# include <string.h>
# include <unistd.h>
# include <support/capture_subprocess.h>

# define TUNABLE_NAMESPACE cpu
# include <elf/dl-tunables.h>

# define STRINGIFY(x) STRINGIFY1 (x)
# define STRINGIFY1(x) #x

/* The full internal name of the tunable, e.g. "glibc.cpu.hwcaps", as
   embedded in the fatal error message.  */
# define TST_SEAL_TUNABLE_FULLNAME \
  STRINGIFY (TOP_NAMESPACE) "." STRINGIFY (TUNABLE_NAMESPACE) "." \
  STRINGIFY (TST_SEAL_TUNABLE_NAME)

static void
read_sealed_tunable (void *closure)
{
  TUNABLE_GET (TST_SEAL_TUNABLE_NAME, struct tunable_str_t *, NULL);
  /* Not reached: the read above is a fatal error.  Exit successfully so
     that a missing diagnostic is caught as an unexpected exit status.  */
  _exit (EXIT_SUCCESS);
}

#endif /* TST_SEAL_TUNABLE_NAME  */

static int
do_test (void)
{
#ifndef TST_SEAL_TUNABLE_NAME
  FAIL_UNSUPPORTED ("the target ABI has no glibc.cpu string tunable");
#else
  struct support_capture_subprocess result
    = support_capture_subprocess (read_sealed_tunable, NULL);

  support_capture_subprocess_check (&result, "tst-tunables-seal", 127,
				    sc_allow_stderr);
  TEST_VERIFY (strstr (result.err.buffer,
		       "Fatal glibc error: " TST_SEAL_TUNABLE_FULLNAME
		       ": string tunable read after process initialization")
	       != NULL);

  support_capture_subprocess_free (&result);
  return 0;
#endif
}

#include <support/test-driver.c>
