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

/* Maximum number of entries in the array passed to futex_waitv.  */
#define FUTEX_WAITV_MAX		128

/* Per-waiter flags for futex_waitv, following the kernel definitions
   from <linux/futex.h>.  FUTEX2_PRIVATE equals FUTEX_PRIVATE_FLAG, so
   the same value is used by all futex_* operations.  */
#ifndef FUTEX2_SIZE_U32
# define FUTEX2_SIZE_U32	0x02
#endif
#ifndef FUTEX2_PRIVATE
# define FUTEX2_PRIVATE		FUTEX_PRIVATE_FLAG
#endif
/* With FUTEX2_NUMA the futex word is followed by a second 32-bit word
   holding the NUMA node id of the kernel state associated with the
   futex; FUTEX_NO_NODE there lets the kernel choose the node.
   FUTEX2_MPOL applies the memory policy of the futex word address.  */
#ifndef FUTEX2_NUMA
# define FUTEX2_NUMA		0x04
#endif
#ifndef FUTEX2_MPOL
# define FUTEX2_MPOL		0x08
#endif
#ifndef FUTEX_NO_NODE
# define FUTEX_NO_NODE		(-1)
#endif

/* One futex to wait on in a futex_waitv call, with the same layout as
   the kernel struct futex_waitv.  */
struct futex_waiter
{
  uint64_t val;			/* Expected value of the futex word.  */
  uint64_t uaddr;		/* Address of the futex word.  */
  uint32_t flags;		/* FUTEX2_* flags for this futex word.  */
  uint32_t __reserved;		/* Must be zero.  */
};

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

/* Block on all of the NWAITERS futex words described by WAITERS (at most
   FUTEX_WAITV_MAX entries) until woken on any of them or until the absolute
   timeout ABSTIME measured against CLOCKID (which must be CLOCK_REALTIME or
   CLOCK_MONOTONIC) passes.  If ABSTIME is null, block without a timeout.
   Each entry checks its own expected value and carries its own FUTEX2_*
   flags (FUTEX2_SIZE_U32, optionally with FUTEX2_PRIVATE).  */
#ifdef __USE_TIME_BITS64
extern int futex_waitv (const struct futex_waiter *__waiters,
			unsigned int __nwaiters, unsigned int __flags,
			const struct timespec *__abstime,
			clockid_t __clockid)
     __THROW __nonnull ((1));
#endif

__END_DECLS

#endif /* sys/futex.h */
