/* Wrappers for the Linux futex system call.
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

#ifndef _SYS_FUTEX_H
#define _SYS_FUTEX_H	1

#include <features.h>
#include <stdint.h>
#include <bits/types.h>
#include <bits/types/clockid_t.h>
#include <bits/types/struct_timespec.h>

#ifndef FUTEX_PRIVATE_FLAG
# define FUTEX_PRIVATE_FLAG	128
#endif

__BEGIN_DECLS

/* If *FUTEXP == EXPECTED block until woken by futex_wake (or spuriously).
   Returns 0 when woken, or -1 and sets errno on failure.  */
extern int futex_wait (uint32_t *__futexp, uint32_t __expected,
		       unsigned int __flags)
     __THROW __nonnull ((1));

/* Like futex_wait, but do not block past the absolute timeout ABSTIME
   measured against CLOCKID (which must be CLOCK_REALTIME or
   CLOCK_MONOTONIC).  If ABSTIME is null, block without a timeout.  */
#ifndef __USE_TIME64_REDIRECTS
extern int futex_timedwait (uint32_t *__futexp, uint32_t __expected,
			    clockid_t __clockid,
			    const struct timespec *__abstime,
			    unsigned int __flags)
     __THROW __nonnull ((1));
#else
# ifdef __REDIRECT_NTH
extern int __REDIRECT_NTH (futex_timedwait,
			   (uint32_t *__futexp, uint32_t __expected,
			    clockid_t __clockid,
			    const struct timespec *__abstime,
			    unsigned int __flags),
			   __futex_timedwait_time64)
     __nonnull ((1));
# else
#  define futex_timedwait __futex_timedwait_time64
extern int futex_timedwait (uint32_t *__futexp, uint32_t __expected,
			    clockid_t __clockid,
			    const struct timespec *__abstime,
			    unsigned int __flags)
     __THROW __nonnull ((1));
# endif
#endif

/* Wake up to COUNT threads blocked on FUTEXP (INT_MAX to wake all waiters).
   Returns the number of woken threads (zero if no thread was blocked), or
   -1 and sets errno on failure.  */
extern int futex_wake (uint32_t *__futexp, int __count, unsigned int __flags)
     __THROW __nonnull ((1));

/* If *FUTEXP == EXPECTED wake up to NWAKE threads blocked on FUTEXP and
   requeue up to NREQUEUE of the remaining blocked threads to wait on TARGETP
   instead.  Returns the total number of woken and requeued threads, or -1
   and sets errno on failure (EAGAIN if *FUTEXP does not match EXPECTED).  */
extern int futex_requeue (uint32_t *__futexp, uint32_t __expected,
			  int __nwake, uint32_t *__targetp, int __nrequeue,
			  unsigned int __flags)
     __THROW __nonnull ((1, 4));

__END_DECLS

#endif /* sys/futex.h */
