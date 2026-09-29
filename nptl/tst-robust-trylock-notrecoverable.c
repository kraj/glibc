/* Test that pthread_mutex_trylock releases an unrecoverable robust mutex.
   Copyright The GNU Toolchain Authors.
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

#include <array_length.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <support/check.h>
#include <support/timespec.h>
#include <support/xthread.h>
#include <support/xtime.h>

static pthread_mutex_t mutex;

static void *
lock_and_exit (void *arg)
{
  TEST_COMPARE (pthread_mutex_lock (&mutex), 0);
  return NULL;
}

static void *
trylock_in_thread (void *arg)
{
  int *result = arg;
  *result = pthread_mutex_trylock (&mutex);
  return NULL;
}

static void
run_test (bool pshared, bool pi, int type)
{
  printf ("info: pshared=%d pi=%d type=%d\n", pshared, pi, type);

  pthread_mutexattr_t attr;
  xpthread_mutexattr_init (&attr);
  xpthread_mutexattr_setrobust (&attr, PTHREAD_MUTEX_ROBUST);
  xpthread_mutexattr_settype (&attr, type);
  if (pshared)
    xpthread_mutexattr_setpshared (&attr, PTHREAD_PROCESS_SHARED);
  if (pi)
    xpthread_mutexattr_setprotocol (&attr, PTHREAD_PRIO_INHERIT);

  int init_result = pthread_mutex_init (&mutex, &attr);
  xpthread_mutexattr_destroy (&attr);
  if (pi && init_result == ENOTSUP)
    {
      puts ("info: PI robust mutexes not supported; skipping this combination");
      return;
    }
  TEST_COMPARE (init_result, 0);
  if (init_result != 0)
    return;

  /* The owner dies, and its successor unlocks without making the mutex
     consistent.  All subsequent lock attempts must fail.  */
  xpthread_join (xpthread_create (NULL, lock_and_exit, NULL));
  TEST_COMPARE (pthread_mutex_lock (&mutex), EOWNERDEAD);
  TEST_COMPARE (pthread_mutex_unlock (&mutex), 0);

  TEST_COMPARE (pthread_mutex_lock (&mutex), ENOTRECOVERABLE);
  TEST_COMPARE (pthread_mutex_trylock (&mutex), ENOTRECOVERABLE);
  /* With a recursive mutex, an incorrectly retained lock can return 0
     rather than EBUSY on a second trylock by the same thread.  */
  TEST_COMPARE (pthread_mutex_trylock (&mutex), ENOTRECOVERABLE);

  int result = -1;
  xpthread_join (xpthread_create (NULL, trylock_in_thread, &result));
  TEST_COMPARE (result, ENOTRECOVERABLE);

  /* A bounded lock attempt detects a lock word retained by trylock.  */
  struct timespec timeout = timespec_add (xclock_now (CLOCK_REALTIME),
					  make_timespec (1, 0));
  TEST_COMPARE (pthread_mutex_timedlock (&mutex, &timeout), ENOTRECOVERABLE);
  TEST_COMPARE (pthread_mutex_destroy (&mutex), 0);
}

static int
do_test (void)
{
  static const int types[] = {
    PTHREAD_MUTEX_NORMAL, PTHREAD_MUTEX_RECURSIVE, PTHREAD_MUTEX_ERRORCHECK
  };

  for (size_t index = 0; index < array_length (types); ++index)
    for (int pshared = 0; pshared < 2; ++pshared)
      for (int pi = 0; pi < 2; ++pi)
	run_test (pshared, pi, types[index]);
  return 0;
}

#include <support/test-driver.c>
