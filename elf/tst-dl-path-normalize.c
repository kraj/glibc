/* Unit tests for dl-path-normalize.h.
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

#include <dl-path-normalize.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <support/check.h>
#include <support/next_to_fault.h>

static void
check_one_guarded (const char *input, const char *expected, bool before)
{
  size_t size = strlen (input) + 1;
  struct support_next_to_fault ntf
    = before ? support_next_to_fault_allocate_before (size)
	     : support_next_to_fault_allocate (size);
  memcpy (ntf.buffer, input, size);

  size_t len = _dl_normalize_path (ntf.buffer);

  TEST_COMPARE (len, strlen (ntf.buffer));
  TEST_COMPARE_STRING (ntf.buffer, expected);

  support_next_to_fault_free (&ntf);
}

static void
check_one (const char *input, const char *expected)
{
  /* Check that _dl_normalize_path does not access the string outside the
     input argument.  It checks for both over-runs and under-runs (the
     latter for the case of '..' expansions).  */
  check_one_guarded (input, expected, false);
  check_one_guarded (input, expected, true);
}

static int
do_test (void)
{
  /* Absolute paths.  */
  check_one ("/", "/");
  check_one ("//", "/");
  check_one ("///", "/");
  check_one ("////", "/");
  check_one ("/a", "/a");
  check_one ("/a/", "/a");
  check_one ("/a//", "/a");
  check_one ("//a//b//", "/a/b");
  check_one ("/.", "/");
  check_one ("/./", "/");
  check_one ("/./a", "/a");
  check_one ("/a/./b", "/a/b");
  check_one ("/a/.", "/a");
  check_one ("/..", "/");
  check_one ("/../", "/");
  check_one ("/../a", "/a");
  check_one ("/a/..", "/");
  check_one ("/a/../", "/");
  check_one ("/a/../..", "/");
  check_one ("/a/../b", "/b");
  check_one ("/a/../../b", "/b");
  check_one ("/a/b/../../c", "/c");
  check_one ("/a/b/../c", "/a/c");
  check_one ("/a/b/c/../..", "/a");
  check_one ("/usr/lib/../lib64", "/usr/lib64");
  check_one ("/usr/lib/../lib64/", "/usr/lib64");
  check_one ("/usr/lib/..//lib64/", "/usr/lib64");
  check_one ("/usr/lib/../../lib64/", "/lib64");

  /* "." and ".." are special only as complete components.  */
  check_one ("/a..", "/a..");
  check_one ("/..a", "/..a");
  check_one ("/a/...", "/a/...");
  check_one ("/.../a", "/.../a");
  check_one ("/a./b", "/a./b");
  check_one (".a", ".a");
  check_one ("a.", "a.");
  check_one ("..a", "..a");
  check_one ("...", "...");

  /* Relative paths.  */
  check_one ("", "");
  check_one (".", "");
  check_one ("./", "");
  check_one ("..", "..");
  check_one ("../", "..");
  check_one ("a", "a");
  check_one ("a/", "a");
  check_one ("a//b", "a/b");
  check_one ("a..", "a..");
  check_one ("./a", "a");
  check_one ("./.", "");
  check_one ("./..", "..");

  check_one ("a/..", "");
  check_one ("a/../", "");
  check_one ("ab/..", "");
  check_one (".a/..", "");
  check_one ("a./..", "");
  check_one (".../..", "");
  check_one ("a/./..", "");
  check_one ("a/b/..", "a");
  check_one ("abc/def/..", "abc");

  /* Appending a component to an emptied relative output must not produce a
     leading '/' (the path must stay relative).  */
  check_one ("a/../b", "b");
  check_one ("a/.././b", "b");
  check_one ("a/../lib64/b", "lib64/b");

  /* Leading ".." components of a relative path are preserved and stack
     instead of cancelling each other; ordinary components may follow and be
     removed again afterwards.  */
  check_one ("../a", "../a");
  check_one ("../..", "../..");
  check_one ("../../..", "../../..");
  check_one ("../../a", "../../a");
  check_one ("../a/..", "..");
  check_one ("../../a/..", "../..");
  check_one ("a/../../b", "../b");
  check_one ("a/b/../../..", "..");

  return 0;
}

#include <support/test-driver.c>
