/* Test robust mutex head list management in case of error returns.
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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <support/check.h>
#include <support/timespec.h>
#include <support/xthread.h>
#include <support/xtime.h>

/* This mutex is attempted to be locked by the thread test.  */
static pthread_mutex_t mutex;

/* This is the futex value when the mutex is locked on the test thread.
   It is used to restore the kernel-expected bit pattern in the mutex
   variable.  */
static __typeof (mutex.__data.__lock) futex_value;

static pthread_barrier_t barrier;

/* This test covers various code paths in the locking implementation.  */
static enum
  {
    /* Invalid nanoseconds value in timeout argument.  */
    test_invalid_nsec,
    /* Timeout before performing the futex operation.  */
    test_early_timeout,
    /* Timeout after performing the futex operation.  */
    test_late_timeout,
  } subtest;

static void *
threadfunc (void *ignored)
{
  /* Capture the expected futex value.  */
  xpthread_mutex_lock (&mutex);
  futex_value = mutex.__data.__lock;
  xpthread_mutex_unlock (&mutex);

  /* Wait for the main thread to lock.  */
  xpthread_barrier_wait (&barrier);
  xpthread_barrier_wait (&barrier);

  switch (subtest)
    {
    case test_invalid_nsec:
      {
        struct timespec t = { 0, -1 };
        TEST_COMPARE (pthread_mutex_timedlock (&mutex, &t), EINVAL);
      }
      break;
    case test_early_timeout:
      {
        /* Time is in the past: times out immediately.  */
        struct timespec t = { -1, 0 };
        TEST_COMPARE (pthread_mutex_timedlock (&mutex, &t), ETIMEDOUT);
      }
      break;
    case test_late_timeout:
      {
        /* Time is in the future: expected to block in the kernel.  */
        struct timespec t;
        xclock_gettime (CLOCK_REALTIME, &t);
        t = timespec_add (t, make_timespec (0, 50 * 1000 * 1000));
        TEST_COMPARE (pthread_mutex_timedlock (&mutex, &t), ETIMEDOUT);
      }
      break;
    }

  /* Tell the main thread that it is safe to destroy the mutex.  */
  xpthread_barrier_wait (&barrier);
  /* Wait for the main thread to destroy the mutex.  */
  xpthread_barrier_wait (&barrier);

  /* Exit the thread and trigger robust list processing.  */
  return NULL;
}

static void
test_one (bool use_pi)
{
  xpthread_barrier_init (&barrier, NULL, 2);

  /* Create the robust mutex.  */
  {
    pthread_mutexattr_t a;
    xpthread_mutexattr_init (&a);
    TEST_COMPARE (pthread_mutexattr_setrobust (&a, PTHREAD_MUTEX_ROBUST), 0);
    if (use_pi)
      xpthread_mutexattr_setprotocol (&a, PTHREAD_PRIO_INHERIT);
    xpthread_mutex_init (&mutex, &a);
    xpthread_mutexattr_destroy (&a);
  }

  pthread_t thr = xpthread_create (NULL, threadfunc, NULL);

  /* Wait for the thread to initialize futex_value.  */
  xpthread_barrier_wait (&barrier);

  /* Prevent the pthread_mutex_timedlock call from succeeding.  */
  xpthread_mutex_lock (&mutex);

  /* Allow the test thread to perform the lock.  */
  xpthread_barrier_wait (&barrier);

  /* Wait for the timeout in the test thread.  */
  xpthread_barrier_wait (&barrier);

  /* Destroy the mutex.  */
  xpthread_mutex_unlock (&mutex);
  xpthread_mutex_destroy (&mutex);

  /* Overwrite the mutex with a distinct bit pattern.  */
  char pattern[sizeof (mutex)];
  memset (&pattern, 0xcc, sizeof (pattern));
  /* Set a futex value that will be acted upon by the kernel.  */
  memcpy (&pattern[offsetof (pthread_mutex_t, __data.__lock)],
          &futex_value, sizeof (futex_value));
  memcpy (&mutex, pattern, sizeof (mutex));

  /* Allow the thread to exit.  */
  xpthread_barrier_wait (&barrier);

  /* Wait for the other thread to exit.  */
  xpthread_join (thr);

  /* Check that the mutex object was not written to.  */
  TEST_COMPARE_BLOB (&mutex, sizeof (mutex), pattern, sizeof (pattern));

  xpthread_barrier_destroy (&barrier);
}

static int
do_test (void)
{
  for (int subtest_int = 0; subtest_int <= test_late_timeout; ++subtest_int)
    for (int use_pi = 0; use_pi < 2; ++use_pi)
      {
        printf ("info: subtest %d, PI %s\n",
                subtest_int, use_pi ? "active" : "inactive");
        subtest = subtest_int;
        test_one (use_pi);
      }

  return 0;
}

#include <support/test-driver.c>
