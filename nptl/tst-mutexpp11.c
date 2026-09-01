/* Test that PP mutex timedlock timeout restores thread priority (bug 34546).
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
#include <pthread.h>
#include <support/check.h>
#include <support/timespec.h>
#include <support/xthread.h>
#include <support/xtime.h>

#include "tst-tpp.h"

static pthread_mutex_t pp_mutex;
static pthread_barrier_t barrier;

static void *
blocker_thread (void *ignored)
{
  xpthread_mutex_lock (&pp_mutex);

  /* Tell the main thread that the mutex is held.  */
  xpthread_barrier_wait (&barrier);

  /* Wait for the main thread to finish its test.  */
  xpthread_barrier_wait (&barrier);

  xpthread_mutex_unlock (&pp_mutex);
  return NULL;
}

/* Used by CHECK_TPP_PRIORITY.  */
static int ret;

static void
test_timedlock (void)
{
  pthread_t thr = xpthread_create (NULL, blocker_thread, NULL);
  xpthread_barrier_wait (&barrier);

  CHECK_TPP_PRIORITY (4, 4);

  /* Already-expired timeout.  */
  static const struct timespec t = { -1, 0 };
  TEST_COMPARE (pthread_mutex_timedlock (&pp_mutex, &t), ETIMEDOUT);

  /* The timeout error path must restore the thread priority.  */
  CHECK_TPP_PRIORITY (4, 4);

  xpthread_barrier_wait (&barrier);
  xpthread_join (thr);
}

static void
test_clocklock (void)
{
  pthread_t thr = xpthread_create (NULL, blocker_thread, NULL);
  xpthread_barrier_wait (&barrier);

  CHECK_TPP_PRIORITY (4, 4);

  /* Already-expired timeout, using CLOCK_REALTIME.  */
  static const struct timespec t = { -1, 0 };
  TEST_COMPARE (pthread_mutex_clocklock (&pp_mutex, CLOCK_REALTIME, &t),
                ETIMEDOUT);

  CHECK_TPP_PRIORITY (4, 4);

  /* Already-expired timeout, using CLOCK_MONOTONIC.  */
  TEST_COMPARE (pthread_mutex_clocklock (&pp_mutex, CLOCK_MONOTONIC, &t),
                ETIMEDOUT);

  CHECK_TPP_PRIORITY (4, 4);

  xpthread_barrier_wait (&barrier);
  xpthread_join (thr);
}

static void
test_timedlock_late (void)
{
  pthread_t thr = xpthread_create (NULL, blocker_thread, NULL);
  xpthread_barrier_wait (&barrier);

  CHECK_TPP_PRIORITY (4, 4);

  /* Timeout in the near future: blocks in the kernel before timing out.  */
  struct timespec t;
  xclock_gettime (CLOCK_REALTIME, &t);
  t = timespec_add (t, make_timespec (0, 50 * 1000 * 1000));
  TEST_COMPARE (pthread_mutex_timedlock (&pp_mutex, &t), ETIMEDOUT);

  CHECK_TPP_PRIORITY (4, 4);

  xpthread_barrier_wait (&barrier);
  xpthread_join (thr);
}

static int
do_test (void)
{
  init_tpp_test ();

  /* Create the PP mutex.  */
  {
    pthread_mutexattr_t ma;
    xpthread_mutexattr_init (&ma);
    xpthread_mutexattr_setprotocol (&ma, PTHREAD_PRIO_PROTECT);
    TEST_COMPARE (pthread_mutexattr_setprioceiling (&ma, 6), 0);
    xpthread_mutex_init (&pp_mutex, &ma);
    xpthread_mutexattr_destroy (&ma);
  }

  xpthread_barrier_init (&barrier, NULL, 2);
  test_timedlock ();
  test_clocklock ();
  test_timedlock_late ();
  xpthread_barrier_destroy (&barrier);

  xpthread_mutex_destroy (&pp_mutex);

  return ret;
}

#include <support/test-driver.c>
