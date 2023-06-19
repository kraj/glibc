/* Check that SIGEV_THREAD notification functions honor cancellation
   (BZ 30558).
   Copyright (C) 2026 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

/* POSIX requires SIGEV_THREAD notifications to behave as if a new thread was
   created for each delivery and thus it must support cancellation.  */

#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <time.h>

#include <support/check.h>

/* Set by the notification function on its first firing, once it has published
   handler_thread and is about to spin; waited on by do_test.  */
static atomic_int handler_started;

/* Set by a later firing, proving the helper survived the cancellation and is
   reused; waited on by do_test.  */
static atomic_int handler_reused;

static pthread_t handler_thread;

static void
on_timer (union sigval sv)
{
  /* Plain static (not thread-local) storage, so it survives the per-firing
     thread state reset and lets us act only on the first firing.  */
  static atomic_int firings;

  if (atomic_fetch_add (&firings, 1) == 0)
    {
      handler_thread = pthread_self ();

      TEST_COMPARE (pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL), 0);
      TEST_COMPARE (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, NULL),
		    0);

      /* Publish handler_thread and signal do_test.  */
      atomic_store_explicit (&handler_started, 1, memory_order_release);

      /* Spin until asynchronously cancelled, and the flag is never cleared.
	 The only way out is the cancellation.  */
      while (atomic_load_explicit (&handler_started, memory_order_relaxed))
	;
    }
  else
    atomic_store_explicit (&handler_reused, 1, memory_order_release);
}

static int
do_test (void)
{
  timer_t timerid;
  struct sigevent ev =
    {
      .sigev_notify = SIGEV_THREAD,
      .sigev_notify_function = on_timer,
    };
  TEST_COMPARE (timer_create (CLOCK_REALTIME, &ev, &timerid), 0);

  /* Periodic so that a firing keeps arriving after the first one is
     cancelled.  */
  struct itimerspec its =
    { .it_value    = { .tv_nsec = 10000000 /* 0.01s */ },
      .it_interval = { .tv_nsec = 10000000 /* 0.01s */ } };
  TEST_COMPARE (timer_settime (timerid, 0, &its, NULL), 0);

  /* Busy wait until the notification function is spinning.  */
  while (atomic_load_explicit (&handler_started, memory_order_acquire) == 0)
    ;

  TEST_COMPARE (pthread_cancel (handler_thread), 0);

  /* If the cancellation is honored the spin is interrupted, the helper resets
     and resumes, and a later firing sets handler_reused.  */
  while (atomic_load_explicit (&handler_reused, memory_order_acquire) == 0)
    ;

  /* The helper survived the cancellation and is still alive.  */
  TEST_COMPARE (pthread_kill (handler_thread, 0), 0);

  struct itimerspec its_stop = { 0 };
  TEST_COMPARE (timer_settime (timerid, 0, &its_stop, NULL), 0);
  TEST_COMPARE (timer_delete (timerid), 0);

  return 0;
}

#define TIMEOUT 10
#include <support/test-driver.c>
