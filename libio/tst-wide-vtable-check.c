/* Test that a corrupted wide vtable index (_IO_wide_data._wide_vtable_index)
   is detected and the process is terminated.  This exercises the hardening
   that closes the House of Apple 2 / FSROP primitive, where the wide vtable
   dispatch was previously unchecked.
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
#include <wchar.h>
#include <string.h>
#include <signal.h>

#include <libioP.h>
#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/support.h>

/* The fatal message printed by __libc_fatal on detection.  */
static const char expected_message[]
  = "Fatal error: glibc detected an invalid stdio handle\n";

/* Open a wide-oriented stream backed by a temporary file and return it.
   The stream is oriented wide so that subsequent wide operations follow
   the wide vtable dispatch path.  */
static FILE *
open_wide_stream (void)
{
  FILE *fp = tmpfile ();
  TEST_VERIFY_EXIT (fp != NULL);
  /* Orient the stream towards wide characters.  */
  TEST_VERIFY_EXIT (fwide (fp, 1) > 0);
  return fp;
}

/* Callback: overwrite the wide vtable index with an out-of-range value
   (as an attacker who can write the field would) and trigger a wide
   operation.  The bounds check in IO_wide_validate_index rejects it and the
   dispatch aborts.  */
static void
corrupt_out_of_range (void *closure)
{
  FILE *fp = open_wide_stream ();
  fp->_wide_data->_wide_vtable_index = IO_VTABLES_NUM + 100;
  /* Force buffer allocation, which dispatches through the wide vtable
     (_IO_WDOALLOCATE and friends).  */
  fputwc (L'x', fp);
  /* Should not be reached.  */
  fclose (fp);
}

/* Callback: a large index, as would result from an attacker overwriting
   the field with a pointer-sized garbage value truncated into the
   index.  Also out of range, so rejected.  */
static void
corrupt_huge_index (void *closure)
{
  FILE *fp = open_wide_stream ();
  fp->_wide_data->_wide_vtable_index = 0x41414141;
  fputwc (L'y', fp);
  fclose (fp);
}

/* Run CALLBACK in a subprocess and require that it terminates with
   SIGABRT and prints the fatal stdio message.  */
static void
expect_termination (const char *name, void (*callback) (void *))
{
  struct support_capture_subprocess proc
    = support_capture_subprocess (callback, NULL);
  support_capture_subprocess_check (&proc, name, -SIGABRT, sc_allow_stderr);
  TEST_COMPARE_BLOB (proc.err.buffer, proc.err.length,
                     expected_message, strlen (expected_message));
  support_capture_subprocess_free (&proc);
}

/* Sanity check: an untampered wide stream works and does not abort.  */
static void
legitimate_stream (void *closure)
{
  FILE *fp = open_wide_stream ();
  TEST_VERIFY (fputwc (L'z', fp) == L'z');
  TEST_VERIFY (fclose (fp) == 0);
}

static int
do_test (void)
{
  /* The legitimate case must run to completion (exit status 0).  */
  {
    struct support_capture_subprocess proc
      = support_capture_subprocess (legitimate_stream, NULL);
    support_capture_subprocess_check (&proc, "legitimate", 0, sc_allow_stderr);
    support_capture_subprocess_free (&proc);
  }

  /* Corruptions must be detected and abort the process.  */
  expect_termination ("out-of-range", corrupt_out_of_range);
  expect_termination ("huge-index", corrupt_huge_index);

  return 0;
}

#include <support/test-driver.c>
