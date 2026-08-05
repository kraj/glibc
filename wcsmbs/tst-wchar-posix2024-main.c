/* Check POSIX.1-2024 declarations in <wchar.h>.
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

#include <wchar.h>

static int
do_test (void)
{
  size_t (*wcslcpy_function) (wchar_t *, const wchar_t *, size_t) = wcslcpy;
  size_t (*wcslcat_function) (wchar_t *, const wchar_t *, size_t) = wcslcat;

#ifdef TEST_FORTIFY
  size_t (*wcslcpy_chk_function) (wchar_t *, const wchar_t *, size_t, size_t)
    = __wcslcpy_chk;
  size_t (*wcslcat_chk_function) (wchar_t *, const wchar_t *, size_t, size_t)
    = __wcslcat_chk;
#endif

  int result = wcslcpy_function == NULL || wcslcat_function == NULL;
#ifdef TEST_FORTIFY
  result |= wcslcpy_chk_function == NULL || wcslcat_chk_function == NULL;
#endif
  return result;
}

#include <support/test-driver.c>
