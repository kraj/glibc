/* Copyright (C) 2005-2025 Free Software Foundation, Inc.
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

#include <locale.h>
#include <string.h>
#include <wchar.h>


size_t
__wcstombs_chk (char *dst, const wchar_t *src, size_t len, size_t dstlen)
{
  if (__glibc_unlikely (dstlen < len))
    __chk_fail ();

  mbstate_t state;

  memset (&state, '\0', sizeof state);

  /* Return how many we wrote (or maybe an error).  */
  return __wcsrtombs (dst, &src, len, &state);
}
