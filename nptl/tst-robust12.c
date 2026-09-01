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


/* Check if the pthread clear pending robut list for the kernel detected
   deadlock cases.  */

static pthread_mutex_t mutex2;

/* Fro the kABI futex interface.  */
#define FUTEX_WAITERS 0x80000000

struct deadlock_args
{
  bool use_timedlock;
  int result;
};

static void *
deadlock_threadfunc (void *closure)
{
  struct deadlock_args *args = closure;

  xpthread_mutex_lock (&mutex);
  futex_value = mutex.__data.__lock;
  xpthread_mutex_unlock (&mutex);
  xpthread_barrier_wait (&barrier);

  xpthread_barrier_wait (&barrier);
  xpthread_mutex_lock (&mutex2);
  xpthread_barrier_wait (&barrier);

  while ((__atomic_load_n (&mutex2.__data.__lock, __ATOMIC_RELAXED)
	  & FUTEX_WAITERS) == 0)
    nanosleep (&(struct timespec) { 0, 100000 }, NULL);

  /* For the case where the kernel has not yet see the main thread as blocked
     (schedule pressure, system load, main thread has timed out, etc), the
     deadload is not reported and the lock attempt succeeds.  So retry for
     such cases.  */
  int ret;
  if (args->use_timedlock)
    {
      /* The timeout is not expected to be triggered, the kernel should
	 report the deadlock.  */
      struct timespec t = timespec_add (xclock_now (CLOCK_REALTIME),
					make_timespec (4, 0));
      ret = pthread_mutex_timedlock (&mutex, &t);
    }
  else
    ret = pthread_mutex_lock (&mutex);
  args->result = ret;
  if (ret == 0)
    xpthread_mutex_unlock (&mutex);

  xpthread_barrier_wait (&barrier);
  xpthread_barrier_wait (&barrier);

  return NULL;
}

enum { no_deadloadk_attempts = 16 };

static void
test_deadlock (bool use_timedlock)
{
  struct deadlock_args args = { .use_timedlock = use_timedlock };

  /* Retry in the case the deadlock is not reported to the test thread.  */
  for (int attempt = 0; attempt < no_deadloadk_attempts; ++attempt)
    {
      xpthread_barrier_init (&barrier, NULL, 2);

      {
	pthread_mutexattr_t a;
	xpthread_mutexattr_init (&a);
	TEST_COMPARE (pthread_mutexattr_setrobust (&a, PTHREAD_MUTEX_ROBUST),
		      0);
	/* NB: error checkin, so that kernel detects the deadlock.  */
	xpthread_mutexattr_settype (&a, PTHREAD_MUTEX_ERRORCHECK);
	xpthread_mutexattr_setprotocol (&a, PTHREAD_PRIO_INHERIT);
	xpthread_mutex_init (&mutex, &a);
	xpthread_mutex_init (&mutex2, &a);
	xpthread_mutexattr_destroy (&a);
      }

      args.result = -1;
      pthread_t thr = xpthread_create (NULL, deadlock_threadfunc, &args);

      xpthread_barrier_wait (&barrier);

      xpthread_mutex_lock (&mutex);
      /* Allow the test thread to lock MUTEX2.  */
      xpthread_barrier_wait (&barrier);
      /* Wait for the test thread to lock MUTEX2.  */
      xpthread_barrier_wait (&barrier);

      struct timespec t = timespec_add (xclock_now (CLOCK_REALTIME),
					make_timespec (2, 0));
      int ret = pthread_mutex_timedlock (&mutex2, &t);
      TEST_VERIFY (ret == ETIMEDOUT || ret == EDEADLK);

      /* In the expected case the target should have received EDEADLK,
         otherwise the kernel reported the deadlock to this thread instead,
	 or the timeout expired efore thre test thread attempted the lock.
	 For the later retry the round.  */
      xpthread_mutex_unlock (&mutex);

      /* Wait for the test thread's lock attempt to finish.  */
      xpthread_barrier_wait (&barrier);

      if (args.result != 0)
	TEST_COMPARE (args.result, EDEADLK);

      /* Destroy MUTEX and overwrite it with a different bit pattern than a
	 valid futex value that will be acted upon by the kernel if the
	 robust list of the exiting test thread still references the mutex.
	 MUTEX2 is still owned by the test thread and is released by the
	 kernel when the thread exits.  */
      xpthread_mutex_destroy (&mutex);
      char pattern[sizeof (mutex)];
      memset (pattern, 0xcc, sizeof (pattern));
      memcpy (&pattern[offsetof (pthread_mutex_t, __data.__lock)],
	      &futex_value, sizeof (futex_value));
      memcpy (&mutex, pattern, sizeof (mutex));

      xpthread_barrier_wait (&barrier);
      xpthread_join (thr);

      TEST_COMPARE_BLOB (&mutex, sizeof (mutex), pattern, sizeof (pattern));

      xpthread_barrier_destroy (&barrier);

      if (args.result == EDEADLK)
	return;
    }
  FAIL_EXIT1 ("deadlock was never reported to the test thread");
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

  for (int use_timedlock = 0; use_timedlock < 2; ++use_timedlock)
    {
      printf ("info: deadlock subtest, %s\n",
	      use_timedlock ? "pthread_mutex_timedlock"
			    : "pthread_mutex_lock");
      test_deadlock (use_timedlock);
    }

  return 0;
}

#include <support/test-driver.c>
