/* Tests for MEMTAG support.
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

#include <dlfcn.h>
#include <string.h>
#include <sys/auxv.h>
#include <support/check.h>

#include "tst-mte-helper.h"

static int
do_test (void)
{
  if (!(getauxval (AT_HWCAP2) & HWCAP2_MTE))
    FAIL_UNSUPPORTED ("kernel or CPU does not support or enable MTE");

  TEST_VERIFY_EXIT (!mte_enable ());

  /* Verify that if MTE is not enabled at startup, trying to load a DSO that
     requires MTE should fail.  */
  void *h = dlopen ("tst-memtag-mod2.so", RTLD_NOW);
  TEST_VERIFY (h == NULL);
  const char *message = dlerror ();
  if (strstr (message, "MTE is not enabled") == 0)
    FAIL_EXIT1 ("invalid dlopen error message");

  return 0;
}

#include <support/test-driver.c>
