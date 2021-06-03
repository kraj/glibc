/* Copyright (C) 2003-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <shlib-compat.h>
#include <sysdep.h>
#include "kernel-posix-timers.h"

extern __typeof (timer_settime) __timer_settime_new;
libc_hidden_proto (__timer_settime_new)

int
___timer_settime_new (timer_t timerid, int flags,
		      const struct itimerspec *value,
		      struct itimerspec *ovalue)
{
  kernel_timer_t ktimerid = timerid_to_kernel_timer (timerid);

  return INLINE_SYSCALL_CALL (timer_settime, ktimerid, flags, value, ovalue);
}
versioned_symbol (libc, ___timer_settime_new, timer_settime, GLIBC_2_34);
libc_hidden_ver (___timer_settime_new, __timer_settime_new)

#if OTHER_SHLIB_COMPAT (librt, GLIBC_2_3_3, GLIBC_2_34)
compat_symbol (librt, ___timer_settime_new, timer_settime, GLIBC_2_3_3);
#endif

#if OTHER_SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_3_3)
int
__timer_settime_old (int timerid, int flags, const struct itimerspec *value,
		     struct itimerspec *ovalue)
{
  return __timer_settime_new (__timer_compat_list[timerid], flags,
			      value, ovalue);
}
compat_symbol (librt, __timer_settime_old, timer_settime, GLIBC_2_2);
#endif
