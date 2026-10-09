/* Test that all waiters wake up when a robust mutex becomes not recoverable.
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

/* The owner of a robust mutex dies, the next owner gets EOWNERDEAD, and while
   it holds the mutex several other threads block on it.  The owner then
   unlocks without calling pthread_mutex_consistent, so the mutex becomes not
   recoverable and every waiter has to return ENOTRECOVERABLE.

   The unlock wakes a single waiter, which acquires the lock with
   FUTEX_WAITERS set and has to wake the next one when it releases it again.  */

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <support/check.h>
#include <support/process_state.h>
#include <support/timespec.h>
#include <support/xthread.h>
#include <support/xtime.h>

enum { nwaiters = 3 };

static pthread_mutex_t mutex;
static bool use_timedlock;

static void *
lock_and_exit (void *arg)
{
  xpthread_mutex_lock (&mutex);
  /* Exit while holding the mutex, so that the next owner sees EOWNERDEAD.  */
  return NULL;
}

static void *
waiter (void *arg)
{
  pid_t *tid = arg;
  __atomic_store_n (tid, gettid (), __ATOMIC_RELEASE);

  int r;
  if (use_timedlock)
    {
      struct timespec timeout = timespec_add (xclock_now (CLOCK_REALTIME),
					      make_timespec (3600, 0));
      r = pthread_mutex_timedlock (&mutex, &timeout);
    }
  else
    r = pthread_mutex_lock (&mutex);
  TEST_COMPARE (r, ENOTRECOVERABLE);
  return NULL;
}

static void
run_test (bool pshared, bool timedlock)
{
  printf ("info: pshared=%d timedlock=%d\n", pshared, timedlock);
  use_timedlock = timedlock;

  pthread_mutexattr_t attr;
  xpthread_mutexattr_init (&attr);
  xpthread_mutexattr_setrobust (&attr, PTHREAD_MUTEX_ROBUST);
  if (pshared)
    xpthread_mutexattr_setpshared (&attr, PTHREAD_PROCESS_SHARED);
  xpthread_mutex_init (&mutex, &attr);
  xpthread_mutexattr_destroy (&attr);

  xpthread_join (xpthread_create (NULL, lock_and_exit, NULL));
  TEST_COMPARE (pthread_mutex_lock (&mutex), EOWNERDEAD);

  pid_t tids[nwaiters] = { 0 };
  pthread_t threads[nwaiters];
  for (int i = 0; i < nwaiters; i++)
    threads[i] = xpthread_create (NULL, waiter, &tids[i]);

  /* Wait until all waiters are blocked on the mutex.  A waiter that has
     published its TID does not sleep anywhere else before blocking.  */
  for (int i = 0; i < nwaiters; i++)
    {
      pid_t tid;
      while ((tid = __atomic_load_n (&tids[i], __ATOMIC_ACQUIRE)) == 0)
	sched_yield ();
      support_thread_state_wait (tid, support_process_state_sleeping);
    }

  /* Make the mutex not recoverable.  */
  TEST_COMPARE (pthread_mutex_unlock (&mutex), 0);

  /* If the wake-up is lost, the remaining waiters block forever and the
     test times out here.  */
  for (int i = 0; i < nwaiters; i++)
    xpthread_join (threads[i]);

  TEST_COMPARE (pthread_mutex_destroy (&mutex), 0);
}

static int
do_test (void)
{
  run_test (false, false);
  run_test (false, true);
  run_test (true, false);
  run_test (true, true);
  return 0;
}

#include <support/test-driver.c>
