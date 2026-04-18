/* Regression test for vfscanf %Nmc out-of-bound write (BZ #34008)
   Copyright (C) 2026 The GNU Toolchain Authors.
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

#include "malloc/mcheck.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <malloc.h>
#include <support/check.h>

#define WIDTH 0x410
#define SCANFSTR "%1040mc"
static int
do_test (void)
{
  mcheck_pedantic (NULL);
  char *input = malloc (WIDTH + 1);
  TEST_VERIFY (input != NULL);
  memset (input, 'A', WIDTH);
  input[WIDTH] = '\0';

  char *buf = NULL;
  TEST_VERIFY (sscanf (input, SCANFSTR, &buf) != -1);
  TEST_VERIFY (buf != NULL);

  free (buf);
  free (input);
  return 0;
}

#include <support/test-driver.c>
