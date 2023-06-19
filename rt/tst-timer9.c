/* Check that timer_getoverrun reports SIGEV_THREAD expirations.
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

#include <signal.h>
#include <stdatomic.h>
#include <time.h>

#include <support/check.h>

static atomic_int handler_started;
static atomic_int keep_spinning = 1;

static void
on_timer (union sigval sv)
{
  static atomic_int firings;

  if (atomic_fetch_add (&firings, 1) == 0)
    {
      atomic_store_explicit (&handler_started, 1, memory_order_release);

      /* Simulate handler stuck under load.  While this runs, further
	 expirations are delivered and folded into the timer overrun.  */
      while (atomic_load_explicit (&keep_spinning, memory_order_acquire))
	;
    }
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

  /* Periodic so expirations keep arriving while the first delivery spins.  */
  enum { interval_nsec = 10000000 }; /* 0.01s */
  struct itimerspec its =
    { .it_value    = { .tv_nsec = interval_nsec },
      .it_interval = { .tv_nsec = interval_nsec } };
  TEST_COMPARE (timer_settime (timerid, 0, &its, NULL), 0);

  /* Busy wait (acquire) until the notification function is spinning.  */
  while (atomic_load_explicit (&handler_started, memory_order_acquire) == 0)
    ;

  /* Let several expirations pile up while the notification is stuck.  */
  enum { wait_intervals = 10 };
  struct timespec delay =
    { .tv_sec = 0, .tv_nsec = wait_intervals * interval_nsec };
  TEST_COMPARE (nanosleep (&delay, NULL), 0);

  /* The overrun is read while the notification is still spinning, so it is
     stable.  */
  int overrun = timer_getoverrun (timerid);
  if (overrun == -1)
    FAIL_EXIT1 ("timer_getoverrun: %m");
  if (overrun == 0)
    FAIL_EXIT1 ("timer_getoverrun returned 0");

  /* Disarm before releasing.  */
  struct itimerspec its_stop = { 0 };
  TEST_COMPARE (timer_settime (timerid, 0, &its_stop, NULL), 0);
  atomic_store_explicit (&keep_spinning, 0, memory_order_release);
  TEST_COMPARE (timer_delete (timerid), 0);

  return 0;
}

#define TIMEOUT 10
#include <support/test-driver.c>
