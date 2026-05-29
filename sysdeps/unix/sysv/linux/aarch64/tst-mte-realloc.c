/* AArch64 tests for heap memory tagging.
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

#include <support/check.h>
#include <support/support.h>
#include <support/xsignal.h>
#include <support/test-driver.h>
#include <array_length.h>

#include "tst-mte-helper.h"

/* Characteristic malloc sizes to cover various allocation methods.  */
size_t sizes[] = {
  1,
  16, 40, 64, 120,
  128, 500, 1000,
  1050, 4096, 5000, 65000,
  131072, 2000000
};

static void check_realloc (size_t len)
{
  /* Tagged pointers.  */
  void *tm, *new_tm;

  printf ("testing realloc (NULL) for req size %zu\n", len);
  tm = realloc (NULL, len);
  check_tags (tm);

  /* Reduce size.  */
  printf ("testing realloc (decreased size) for req size %zu\n", len);
  new_tm = realloc (tm, len / 2 + 1);
  check_tags (new_tm);

  /* Increase size.  */
  printf ("testing realloc (increased size) for req size %zu\n", len);
  new_tm = realloc (new_tm, len + 2);
  check_tags (new_tm);

  free (new_tm);
}

static int
do_test (void)
{

  /* Check if MTE is supported, configured and enabled.  */
  check_mte_enabled ();

  array_foreach_const (plen, sizes)
    check_realloc (*plen);

  return 0;
}

#include <support/test-driver.c>

