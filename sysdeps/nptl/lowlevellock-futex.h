/* Low-level locking access to futex facilities.  Stub version.
   Copyright (C) 2014-2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _LOWLEVELLOCK_FUTEX_H
#define _LOWLEVELLOCK_FUTEX_H   1

#ifndef __ASSEMBLER__
# include <sysdep.h>
# include <sysdep-cancel.h>
# include <kernel-features.h>
# include <time.h>
#endif

#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_REQUEUE		3
#define FUTEX_CMP_REQUEUE	4
#define FUTEX_WAKE_OP		5
#define FUTEX_OP_CLEAR_WAKE_IF_GT_ONE	((4 << 24) | 1)
#define FUTEX_LOCK_PI		6
#define FUTEX_UNLOCK_PI		7
#define FUTEX_TRYLOCK_PI	8
#define FUTEX_WAIT_BITSET	9
#define FUTEX_WAKE_BITSET	10
#define FUTEX_WAIT_REQUEUE_PI   11
#define FUTEX_CMP_REQUEUE_PI    12
#define FUTEX_PRIVATE_FLAG	128
#define FUTEX_CLOCK_REALTIME	256

#define FUTEX_BITSET_MATCH_ANY	0xffffffff

/* Values for 'private' parameter of locking macros.  Yes, the
   definition seems to be backwards.  But it is not.  The bit will be
   reversed before passing to the system call.  */
#define LLL_PRIVATE	0
#define LLL_SHARED	FUTEX_PRIVATE_FLAG

#ifndef __ASSEMBLER__

static inline int
__lll_private_flag (int fl, int priv)
{
/* In libc.so or ld.so all futexes are private.  */
# if IS_IN (libc) || IS_IN (rtld)
  return fl | FUTEX_PRIVATE_FLAG;
# else
  return (fl | FUTEX_PRIVATE_FLAG) ^ priv;
# endif
}

/* For most of these macros, the return value is never really used.
   Nevertheless, the protocol is that each one returns a negated errno
   code for failure or zero for success.  (Note that the corresponding
   Linux system calls can sometimes return positive values for success
   cases too.  We never use those values.)  */


/* Wait while *FUTEXP == VAL for an lll_futex_wake call on FUTEXP.  */
static inline int
lll_futex_timed_wait (int *futexp, int val, const struct timespec *ts,
		      int priv)
{
  int r = internal_syscall (__NR_futex, futexp,
			    __lll_private_flag (FUTEX_WAIT, priv),
			    val, ts);
  return __syscall_err (r) ? r : 0;
}

static inline int
lll_futex_wait (int *futexp, int val, int priv)
{
  return lll_futex_timed_wait (futexp, val, NULL, priv);
}

/* Verify whether the supplied clockid is supported by
   lll_futex_clock_wait_bitset.  */
# define lll_futex_supported_clockid(clockid)			\
  ((clockid) == CLOCK_REALTIME || (clockid) == CLOCK_MONOTONIC)

/* The kernel currently only supports CLOCK_MONOTONIC or
   CLOCK_REALTIME timeouts for FUTEX_WAIT_BITSET.  We could attempt to
   convert others here but currently do not.  */
static inline int
lll_futex_clock_wait_bitset (int *futexp, int val, int clockid,
			     const struct timespec *ts, int priv)
{
  if (! lll_futex_supported_clockid (clockid))
    return -EINVAL;

  const unsigned int clockbit = clockid == CLOCK_REALTIME
				? FUTEX_CLOCK_REALTIME : 0;
  const int op = __lll_private_flag (FUTEX_WAIT_BITSET | clockbit, priv);
  int r =  internal_syscall (__NR_futex, futexp, op, val, ts, NULL,
			     FUTEX_BITSET_MATCH_ANY);
  return __syscall_err (r) ? r : 0;
}


/* Wake up up to NR waiters on FUTEXP.  */
static inline int
lll_futex_wake (int *futexp, int val, int priv)
{
  int r = internal_syscall (__NR_futex, futexp,
			    __lll_private_flag (FUTEX_WAKE, priv), val);
  return __syscall_err (r) ? r : 0;
}

/* Wake up up to NR_WAKE waiters on FUTEXP.  Move up to NR_MOVE of the
   rest from waiting on FUTEXP to waiting on MUTEX (a different futex).
   Returns non-zero if error happened, zero if success.  */
static inline int
lll_futex_requeue (int *futexp, int nr_wake, int nr_move,
		   int *mutex, int val, int priv)
{
  int r = internal_syscall (__NR_futex, futexp,
			    __lll_private_flag (FUTEX_CMP_REQUEUE, priv),
			    nr_wake, nr_move, mutex, val);
  return __syscall_err (r) ? r : 0;
}

/* Wake up up to NR_WAKE waiters on FUTEXP and NR_WAKE2 on FUTEXP2.
   Returns non-zero if error happened, zero if success.  */
static inline int
lll_futex_wake_unlock (int *futexp, int nr_wake, int nr_wake2, int *futexp2,
		       int priv)
{
  int r = internal_syscall (__NR_futex, futexp,
			    __lll_private_flag (FUTEX_WAKE_OP, priv),
			    nr_wake, nr_wake2, futexp2,
			    FUTEX_OP_CLEAR_WAKE_IF_GT_ONE);
  return __syscall_err (r) ? r : 0;
}


/* Priority Inheritance support.  */
static inline int
lll_futex_timed_lock_pi (int *futexp, const struct timespec *ts, int priv)
{
  int r = internal_syscall (__NR_futex, futexp,
			    __lll_private_flag (FUTEX_LOCK_PI, priv),
			    0, ts);
  return __syscall_err (r) ? r : 0;
}

static inline int
lll_futex_trylock_pi (int *futexp, int priv)
{
  int r = internal_syscall (__NR_futex, futexp,
			    __lll_private_flag (FUTEX_TRYLOCK_PI, priv),
			    0, 0);
  return __syscall_err (r) ? r : 0;
}

static inline int
lll_futex_timed_unlock_pi (int *futexp, int priv)
{
  int r = internal_syscall (__NR_futex, futexp,
			    __lll_private_flag (FUTEX_UNLOCK_PI, priv),
			    NULL);
  return __syscall_err (r) ? r : 0;
}

/* Like lll_futex_wait_requeue_pi, but with a timeout.  */
static inline int
lll_futex_timed_wait_requeue_pi (int *futexp, int val,
				 const struct timespec *ts, int clockbit,
				 int *mutex, int priv)
{
  int r = internal_syscall (__NR_futex,
			    __lll_private_flag (FUTEX_WAIT_REQUEUE_PI
						| clockbit, priv),
			    val, ts, mutex);
  return __syscall_err (r) ? r : 0;
}

/* Like lll_futex_wait (FUTEXP, VAL, PRIVATE) but with the expectation
   that lll_futex_cmp_requeue_pi (FUTEXP, _, _, MUTEX, _, PRIVATE) will
   be used to do the wakeup.  Confers priority-inheritance behavior on
   the waiter.  */
static inline int
lll_futex_wait_requeue_pi (int *futexp, int val, int *mutex, int priv)
{
  return lll_futex_timed_wait_requeue_pi (futexp, val, NULL, 0, mutex, priv);
}

/* Like lll_futex_requeue, but pairs with lll_futex_wait_requeue_pi
   and inherits priority from the waiter.  */
static inline int
lll_futex_cmp_requeue_pi (int *futexp, int nr_wake, int nr_move, int *mutex,
			  int val, int priv)
{
  int r = internal_syscall (__NR_futex,
			    __lll_private_flag (FUTEX_CMP_REQUEUE_PI, priv),
			    nr_wake, nr_move, mutex, val);
  return __syscall_err (r) ? r : 0;
}

/* Like lll_futex_wait, but acting as a cancellable entrypoint.  */
# define lll_futex_wait_cancel(futexp, val, private) \
  ({                                                                   \
    int __oldtype = CANCEL_ASYNC ();				       \
    long int __err = lll_futex_wait (futexp, val, LLL_SHARED);	       \
    CANCEL_RESET (__oldtype);					       \
    __err;							       \
  })

/* Like lll_futex_timed_wait, but acting as a cancellable entrypoint.  */
# define lll_futex_timed_wait_cancel(futexp, val, timeout, private) \
  ({									   \
    int __oldtype = CANCEL_ASYNC ();				       	   \
    long int __err = lll_futex_timed_wait (futexp, val, timeout, private); \
    CANCEL_RESET (__oldtype);						   \
    __err;								   \
  })

#endif  /* !__ASSEMBLER__  */

#endif  /* lowlevellock-futex.h */
