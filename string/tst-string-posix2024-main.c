/* Check POSIX.1-2024 declarations in <string.h>.
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

#include <string.h>

static int
do_test (void)
{
  void *(*memmem_function) (const void *, size_t, const void *, size_t)
    = memmem;
  size_t (*strlcpy_function) (char *, const char *, size_t) = strlcpy;
  size_t (*strlcat_function) (char *, const char *, size_t) = strlcat;

#ifdef TEST_FORTIFY
  size_t (*strlcpy_chk_function) (char *, const char *, size_t, size_t)
    = __strlcpy_chk;
  size_t (*strlcat_chk_function) (char *, const char *, size_t, size_t)
    = __strlcat_chk;
#endif

  int result = memmem_function == NULL
    || strlcpy_function == NULL
    || strlcat_function == NULL;
#ifdef TEST_FORTIFY
  result |= strlcpy_chk_function == NULL || strlcat_chk_function == NULL;
#endif
  return result;
}

#include <support/test-driver.c>
