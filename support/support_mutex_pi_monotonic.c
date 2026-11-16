/* Returns whether Priority Inheritance support CLOCK_MONOTONIC.
   Copyright (C) 2021-2026 Free Software Foundation, Inc.
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

#include <support/xthread.h>
#include <sys/syscall.h>
#include <unistd.h>

bool
support_mutex_pi_monotonic (void)
{
#ifdef __linux__
  /* FUTEX_LOCK_PI2 (Linux 5.14) is newer than futex_time64 (Linux 5.1), so
     prefer the latter where it exists.  */
# ifdef __NR_futex_time64
  long int nr = __NR_futex_time64;
# else
  long int nr = __NR_futex;
# endif
  int r = syscall (nr, &(unsigned int){0}, 13 /* FUTEX_LOCK_PI2  */,
		   0, NULL);
  return r == 0;
#else
  return false;
#endif
}
