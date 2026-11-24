/* Test for __wprintf_buffer (bug 35044).
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

#include <printf.h>
#include <stdio.h>
#include <wchar.h>

#include <support/check.h>
#include <support/xstdio.h>

static int
print_w (FILE *fp, const struct printf_info *info, const void *const *args)
{
  for (int i = 0; i < 2000; i++)
    {
      wint_t r = fputwc (L'λ', fp);
      TEST_COMPARE (r, L'λ');
    }
  return 2000;
}

static int
arginfo (const struct printf_info *info, size_t n, int *type, int *size)
{
  return 0;
}

static int
do_test (void)
{
  register_printf_specifier ('W', print_w, arginfo);

  wchar_t *buf = NULL;
  size_t len = 0;
  FILE *f = open_wmemstream (&buf, &len);
  TEST_VERIFY_EXIT (f != NULL);
  int n = fwprintf (f, L"%W");
  TEST_COMPARE (n, 2000);
  xfclose (f);
  TEST_COMPARE (len, 2000);
  for (size_t i = 0; i < len; i++)
    TEST_VERIFY (buf[i] == L'λ');
  return 0;
}

#include <support/test-driver.c>
