/* Test for futex_wait, futex_timedwait, futex_wake, and futex_requeue.
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
#include <limits.h>
#include <sched.h>
#include <stdint.h>
#include <sys/futex.h>
#include <sys/wait.h>
#include <unistd.h>
#include <support/check.h>
#include <support/support.h>
#include <support/timespec.h>
#include <support/xthread.h>
#include <support/xtime.h>
#include <support/xunistd.h>

#define BAD_FLAGS (~FUTEX_PRIVATE_FLAG)

static uint32_t ftx1;
static uint32_t ftx2;

static void *
waiter_tf (void *closure)
{
  while (__atomic_load_n (&ftx1, __ATOMIC_RELAXED) == 0)
    {
      int r = futex_wait (&ftx1, 0, FUTEX_PRIVATE_FLAG);
      TEST_VERIFY_EXIT (r == 0
			|| (r == -1 && (errno == EAGAIN || errno == EINTR)));
    }
  return NULL;
}

static int
do_test (void)
{
  uint32_t word = 5;

  /* Value mismatch does not block.  */
  TEST_COMPARE (futex_wait (&word, 4, FUTEX_PRIVATE_FLAG), -1);
  TEST_COMPARE (errno, EAGAIN);
  TEST_COMPARE (futex_wait (&word, 4, 0), -1);
  TEST_COMPARE (errno, EAGAIN);

  /* Waking with no waiters reports zero woken threads.  */
  TEST_COMPARE (futex_wake (&word, INT_MAX, FUTEX_PRIVATE_FLAG), 0);

  /* Invalid flags.  */
  TEST_COMPARE (futex_wait (&word, 5, BAD_FLAGS), -1);
  TEST_COMPARE (errno, EINVAL);
  TEST_COMPARE (futex_wake (&word, 1, BAD_FLAGS), -1);
  TEST_COMPARE (errno, EINVAL);
  TEST_COMPARE (futex_requeue (&word, 5, 0, &ftx2, 1, BAD_FLAGS), -1);
  TEST_COMPARE (errno, EINVAL);

  /* Requeue with a value mismatch fails and moves no waiters.  */
  TEST_COMPARE (futex_requeue (&ftx1, 1, 0, &ftx2, 1, FUTEX_PRIVATE_FLAG),
		-1);
  TEST_COMPARE (errno, EAGAIN);

  /* Block a thread on FTX1, requeue it to FTX2, and wake it there.  The
     requeue loop also synchronizes with the waiter actually entering the
     kernel, it moves no thread (and returns 0) until the waiter is enqueued
     on FTX1.  */
  {
    pthread_t thr = xpthread_create (NULL, waiter_tf, NULL);

    int r;
    while ((r = futex_requeue (&ftx1, 0, 0, &ftx2, 1, FUTEX_PRIVATE_FLAG))
	   == 0)
      sched_yield ();
    TEST_COMPARE (r, 1);

    /* Release the waiter loop, then wake the requeued thread on FTX2.  */
    __atomic_store_n (&ftx1, 1, __ATOMIC_RELAXED);
    TEST_COMPARE (futex_wake (&ftx2, INT_MAX, FUTEX_PRIVATE_FLAG), 1);
    /* In case the waiter was woken spuriously and blocked on FTX1 again.  */
    futex_wake (&ftx1, INT_MAX, FUTEX_PRIVATE_FLAG);

    xpthread_join (thr);
  }

  /* The same wait/requeue/wake round-trip with process-shared futexes
     (FLAGS of zero) across processes, with the futex words in a shared
     mapping.  */
  {
    uint32_t *sftx = support_shared_allocate (2 * sizeof (uint32_t));

    pid_t pid = xfork ();
    if (pid == 0)
      {
	while (__atomic_load_n (&sftx[0], __ATOMIC_RELAXED) == 0)
	  {
	    int r = futex_wait (&sftx[0], 0, 0);
	    if (r != 0 && !(r == -1 && (errno == EAGAIN || errno == EINTR)))
	      _exit (2);
	  }
	_exit (0);
      }

    /* The requeue loop also synchronizes with the child actually being
       enqueued on the shared futex.  */
    int r;
    while ((r = futex_requeue (&sftx[0], 0, 0, &sftx[1], 1, 0)) == 0)
      sched_yield ();
    TEST_COMPARE (r, 1);

    __atomic_store_n (&sftx[0], 1, __ATOMIC_RELAXED);
    TEST_COMPARE (futex_wake (&sftx[1], INT_MAX, 0), 1);
    /* In case the child was woken spuriously and blocked again.  */
    futex_wake (&sftx[0], INT_MAX, 0);

    int status;
    xwaitpid (pid, &status, 0);
    TEST_VERIFY (WIFEXITED (status));
    TEST_COMPARE (WEXITSTATUS (status), 0);

    support_shared_free (sftx);
  }

  {
    uint32_t tword = 0;
    struct timespec ts = make_timespec (0, 0);

    /* Value mismatch is checked before the timeout.  */
    TEST_COMPARE (futex_timedwait (&word, 4, CLOCK_MONOTONIC, &ts,
				   FUTEX_PRIVATE_FLAG), -1);
    TEST_COMPARE (errno, EAGAIN);

    /* Invalid flags and clocks.  */
    TEST_COMPARE (futex_timedwait (&tword, 0, CLOCK_MONOTONIC, &ts,
				   BAD_FLAGS), -1);
    TEST_COMPARE (errno, EINVAL);
    TEST_COMPARE (futex_timedwait (&tword, 0, CLOCK_PROCESS_CPUTIME_ID, &ts,
				   FUTEX_PRIVATE_FLAG), -1);
    TEST_COMPARE (errno, EINVAL);

    /* Timeouts in the past, including negative tv_sec.  */
    TEST_COMPARE (futex_timedwait (&tword, 0, CLOCK_MONOTONIC, &ts,
				   FUTEX_PRIVATE_FLAG), -1);
    TEST_COMPARE (errno, ETIMEDOUT);
    ts = make_timespec (-1, 0);
    TEST_COMPARE (futex_timedwait (&tword, 0, CLOCK_REALTIME, &ts,
				   FUTEX_PRIVATE_FLAG), -1);
    TEST_COMPARE (errno, ETIMEDOUT);

    /* An actual short wait on both supported clocks.  */
    for (int i = 0; i < 2; i++)
      {
	clockid_t clockid = i == 0 ? CLOCK_REALTIME : CLOCK_MONOTONIC;
	struct timespec timeout
	  = timespec_add (xclock_now (clockid), make_timespec (0, 100000000));
	TEST_COMPARE (futex_timedwait (&tword, 0, clockid, &timeout,
				       FUTEX_PRIVATE_FLAG), -1);
	TEST_COMPARE (errno, ETIMEDOUT);
	TEST_TIMESPEC_NOW_OR_AFTER (clockid, timeout);
      }
  }

  return 0;
}

#include <support/test-driver.c>
