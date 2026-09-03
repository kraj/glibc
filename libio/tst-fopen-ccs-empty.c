/* Test fopen with an empty ",ccs=" value in the mode string (bug 34574).
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <support/check.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/xunistd.h>

static void
check_fopen_fails (const char *path, const char *mode)
{
  errno = 0;
  FILE *fp = fopen (path, mode);
  TEST_VERIFY (fp == NULL);
  TEST_COMPARE (errno, EINVAL);
  if (fp != NULL)
    fclose (fp);
}

static int
do_test (void)
{
  char *path;
  xclose (create_temp_file ("tst-fopen-ccs-empty", &path));

  /* The value is blank and the mode string continues well past it.  */
  enum { size = 1024 * 1024 };
  char *mode = xmalloc (size);
  memset (mode, 'X', size);
  mode[size - 1] = '\0';
  static const char prefix[] = "w,ccs=                     ,";
  memcpy (mode, prefix, sizeof (prefix) - 1);
  check_fopen_fails (path, mode);
  free (mode);

  check_fopen_fails (path, "w,ccs=");
  check_fopen_fails (path, "w,ccs=,");

  free (path);
  return 0;
}

#include <support/test-driver.c>
