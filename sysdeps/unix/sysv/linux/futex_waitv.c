/* Wait on multiple futexes.
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
#include <time.h>
#include <sys/futex.h>
#include <sysdep.h>

int
futex_waitv (const struct futex_waiter *waiters, unsigned int nwaiters,
	     unsigned int flags, const struct __timespec64 *abstime,
	     clockid_t clockid)
{
  int r;
  if (__glibc_unlikely (abstime != NULL && abstime->tv_sec < 0))
    r = -ETIMEDOUT;
  else
    r = INTERNAL_SYSCALL_CALL (futex_waitv, waiters, nwaiters, flags,
			       abstime, clockid);
  return INTERNAL_SYSCALL_ERROR_P (r)
         ? INLINE_SYSCALL_ERROR_RETURN_VALUE (-r)
         : r;
}
