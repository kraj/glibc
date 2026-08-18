/* Wait on a futex.  Linux implementation.
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
futex_wait (uint32_t *futexp, uint32_t expected, unsigned int flags)
{
  if (!__futex_check_flags (flags))
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* A null ABSTIME blocks indefinitely, making this equivalent to a
     plain FUTEX_WAIT.  */
  return __futex_abstimed_wait64_errno (futexp, expected, CLOCK_MONOTONIC,
					NULL, flags);
}
