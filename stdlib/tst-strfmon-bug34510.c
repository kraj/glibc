/* Test handling of right-padding in strfmon (bug 34510, CVE-2026-19499).
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

#include <monetary.h>
#include <errno.h>
#include <support/check.h>
#include <support/next_to_fault.h>

static int
do_test (void)
{
  struct support_next_to_fault ntf = support_next_to_fault_allocate (100);
  TEST_COMPARE (strfmon (ntf.buffer, ntf.length, "%100n", 1.23), -1);
  TEST_COMPARE (errno, E2BIG);
  return 0;
}

#include <support/test-driver.c>
