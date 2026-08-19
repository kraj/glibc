/* Test strftime with a large time zone name.
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

#include <time.h>

#include <libc-diag.h>
#include <limits.h>
#include <stddef.h>
#include <support/blob_repeat.h>
#include <support/check.h>

static int
do_test (void)
{
  /* 6 is chosen so that the truncated value triggered the original bug.  */
  enum { size = (size_t) INT_MAX + 6 };
  struct support_blob_repeat repeat = support_blob_repeat_allocate
    ("X", 1, size);
  if (repeat.start == NULL)
    FAIL_UNSUPPORTED ("could not allocate buffer for time zone name");
  char *tzname = repeat.start;
  tzname[size - 1] = '\0';

  /* Time at the epoch with a fake time zone name.  */
  struct tm tmbuf =
    {
      .tm_mday = 1,
      .tm_zone = tzname,
    };

  char buf[10];
  buf[9] = 'A';

  TEST_COMPARE (strftime (buf, 9, "%Z", &tmbuf), 0);
  TEST_COMPARE (buf[9], 'A');

  /* GCC complains about using a width (3) below with %Z, but it is
     supported in the glibc implementation.  */
  DIAG_PUSH_NEEDS_COMMENT;
  DIAG_IGNORE_NEEDS_COMMENT (0, "-Wformat");

  TEST_COMPARE (strftime (buf, 9, "%3Z", &tmbuf), 0);
  TEST_COMPARE (buf[9], 'A');

  TEST_COMPARE (strftime (buf, 9, "%03Z", &tmbuf), 0);
  TEST_COMPARE (buf[9], 'A');

  DIAG_POP_NEEDS_COMMENT;

  support_blob_repeat_free (&repeat);
  return 0;
}

#include <support/test-driver.c>
