/* Wait on a futex with a timeout.  Linux implementation.
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
#include <futex-internal.h>

int
__futex_timedwait_time64 (uint32_t *futexp, uint32_t expected,
			  clockid_t clockid,
			  const struct __timespec64 *abstime,
			  unsigned int flags)
{
  if (!__futex_check_flags (flags))
    {
      __set_errno (EINVAL);
      return -1;
    }

  return __futex_abstimed_wait64_errno (futexp, expected, clockid, abstime,
					flags);
}

#if __TIMESIZE != 64
libc_hidden_def (__futex_timedwait_time64)

int
futex_timedwait (uint32_t *futexp, uint32_t expected, clockid_t clockid,
		 const struct timespec *abstime, unsigned int flags)
{
  struct __timespec64 ts64, *pts64 = NULL;
  if (abstime != NULL)
    {
      ts64 = valid_timespec_to_timespec64 (*abstime);
      pts64 = &ts64;
    }
  return __futex_timedwait_time64 (futexp, expected, clockid, pts64, flags);
}
#endif
