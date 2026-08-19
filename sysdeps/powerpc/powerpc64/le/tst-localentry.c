/* Check the message for a local entry point mismatch (bug 34541).
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
#include <support/check.h>

#include "tst-localentry-name.h"

static int
do_test (void)
{
  void *h = dlopen ("tst-localentry-mod.so", RTLD_NOW);
  TEST_VERIFY (h == NULL);

  const char *msg = dlerror ();
  TEST_VERIFY_EXIT (msg != NULL);

  TEST_VERIFY (strstr (msg, STR (LOCALENTRY_SYMBOL)) != NULL);
  TEST_VERIFY (strstr (msg, "expected localentry:0") != NULL);

  return 0;
}

#include <support/test-driver.c>
