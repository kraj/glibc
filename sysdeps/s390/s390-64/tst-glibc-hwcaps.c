/* glibc-hwcaps subdirectory test.  s390x version.
   Copyright (C) 2020 Free Software Foundation, Inc.
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
#include <string.h>
#include <support/check.h>
#include <sys/auxv.h>
#include <sys/param.h>

extern int marker2 (void);
extern int marker3 (void);
extern int marker4 (void);

/* Return the POWER level, 8 for the baseline.  */
static int
compute_level (void)
{
  const char *platform = (const char *) getauxval (AT_PLATFORM);

  int result;
  if (sscanf (platform, "arch%d", &result) == 1)
     return result;

  /* The arch* versions refer to the edition of the Principles of
     Operation, and they are off by two when compared with the recent
     product names.  (The code below should not be considered an
     accurate mapping to Principles of Operation editions for earlier
     AT_PLATFORM strings).  */
  if (strcmp (platform, "z900") == 0)
    return 5;
  if (strcmp (platform, "z990") == 0)
    return 6;
  if (strcmp (platform, "z9-109") == 0)
    return 7;
  if (strcmp (platform, "z10") == 0)
    return 8;
  if (strcmp (platform, "z196") == 0)
    return 9;
  if (strcmp (platform, "zEC12") == 0)
    return 10;
  if (strcmp (platform, "z13") == 0)
    return 11;
  if (strcmp (platform, "z14") == 0)
    return 12;
  if (strcmp (platform, "z15") == 0)
    return 13;
  printf ("warning: unrecognized AT_PLATFORM value: %s\n", platform);
  /* Assume that the new platform supports z15.  */
  return 13;
}

static int
do_test (void)
{
  int level = compute_level ();
  printf ("info: detected architecture level: arch%d\n", level);
  TEST_COMPARE (marker2 (), MIN (level - 9, 2));
  TEST_COMPARE (marker3 (), MIN (level - 9, 3));
  TEST_COMPARE (marker4 (), MIN (level - 9, 4));
  return 0;
}

#include <support/test-driver.c>
