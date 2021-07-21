/* Check dladdr with the reference to own exectuable.
   Copyright (C) 2021 Free Software Foundation, Inc.
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
#include <stdint.h>
#include <support/check.h>
#include <support/xdlfcn.h>

void
test_symbol (void)
{
}

static int
do_test (void)
{
  void *handle = xdlopen (NULL, RTLD_NOW);
  int (*sym)(void) = xdlsym (handle, "test_symbol");

  Dl_info info = { 0 };
  {
    int r = dladdr (sym, &info);
    TEST_VERIFY_EXIT (r != 0);
  }

  {
    const char *p = strrchr (info.dli_fname, '/');
    const char *dli_name = p == NULL ? info.dli_fname : p + 1;

    TEST_COMPARE_STRING (dli_name, "tst-dladdr-self");
  }

  TEST_COMPARE_STRING (info.dli_sname, "test_symbol");
  TEST_COMPARE ((uintptr_t) info.dli_saddr, (uintptr_t) test_symbol);

  return 0;
}

#include <support/test-driver.c>
