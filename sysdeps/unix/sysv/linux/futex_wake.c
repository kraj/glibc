/* Wake threads waiting on a futex.  Linux implementation.
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

#include <errno.h>
#include <sys/futex.h>
#include <futex-internal.h>

int
futex_wake (uint32_t *futexp, int count, unsigned int flags)
{
  int r;
  if (__futex_check_flags (flags))
    r = __futex_wake_internal (futexp, count, flags ^ FUTEX_PRIVATE_FLAG);
  else
    r = -EINVAL;

  if (__glibc_unlikely (r < 0))
    {
      __set_errno (-r);
      return -1;
    }
  return r;
}
