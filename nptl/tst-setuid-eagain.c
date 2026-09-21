/* Test that a multithreaded setxid does not silently give up when the
   realtime signal queue is exhausted (bug 21108).
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

/* setxid is implemented on Linux with a SIGSETXID broadcast, and tgkill
   might fail with EAGAIN if the signal queue is full (RLIMIT_SIGPENDING)
   because SIGSETXID is a realtime signal.

   The test first fills the queue, runs setresuid in a helper thread, and
   verifies that it does *not* complete while the queue is still full.

   The test checks whether the signal delivery works as intended, so there is
   no need to change privileges.  A no-op setresuid still broadcasts SIGSETXID
   to the other threads, and with a full queue, the call may only return once
   the queue drains.  */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#include <support/check.h>
#include <support/xthread.h>
#include <support/xunistd.h>

/* How long to wait for the (no-op) setxid to complete before concluding it
   is correctly blocking on the full queue.  It should not return until the
   queue is drained.  */
#define BLOCK_WAIT_SECONDS 2

static pthread_barrier_t start_barrier;
static sem_t setxid_done;
static sem_t worker_exit;

/* A plain thread that must be signalled by the setxid broadcast.  It simply
   parks, remaining in the thread list so it is a delivery target.  */
static void *
worker_thread (void *closure)
{
  xpthread_barrier_wait (&start_barrier);
  while (sem_wait (&worker_exit) != 0)
    /* Restart if interrupted by the SIGSETXID handler.  */;
  return NULL;
}

/* setresuid to the current ids is a no-op but still triggers the full
   SIGSETXID broadcast to the other threads.  */
static void *
setxid_thread (void *closure)
{
  xpthread_barrier_wait (&start_barrier);
  uid_t uid = getuid ();
  TEST_VERIFY_EXIT (setresuid (uid, uid, uid) == 0);
  TEST_VERIFY_EXIT (sem_post (&setxid_done) == 0);
  return NULL;
}

/* The RLIMIT_SIGPENDING accounting is shared by all the processes of the same
   real user ID within a user namespace.  If another process of the same user
   dequeues one of its pending signals while the setxid is expected to block,
   a tgkill succeeds and the setxid returns early.
   Move the test to a new user namespace so the accounting only covers the
   signals queued by the test itself.  It must be called while the process is
   still single-threaded.  */
static void
isolate_sigpending (void)
{
  uid_t uid = getuid ();
  if (unshare (CLONE_NEWUSER) != 0)
    FAIL_UNSUPPORTED ("unable to create a user namespace: %m");

  /* Map the original user ID, so the setresuid operates on a valid ID.  The
     identity mapping also keeps the kernel user ID unchanged, so the test
     driver can still signal this process on timeout.  */
  char buf[64];
  int len = snprintf (buf, sizeof (buf), "%llu %llu 1\n",
		      (unsigned long long int) uid,
		      (unsigned long long int) uid);
  TEST_VERIFY_EXIT (len > 0 && len < sizeof (buf));
  int fd = xopen ("/proc/self/uid_map", O_WRONLY, 0);
  xwrite (fd, buf, len);
  xclose (fd);
}

static int
do_test (void)
{
  isolate_sigpending ();

  TEST_COMPARE (sem_init (&setxid_done, 0, 0), 0);
  TEST_COMPARE (sem_init (&worker_exit, 0, 0), 0);
  xpthread_barrier_init (&start_barrier, NULL, 3);

  sigset_t set;
  sigemptyset (&set);
  sigaddset (&set, SIGRTMIN);
  TEST_COMPARE (pthread_sigmask (SIG_BLOCK, &set, NULL), 0);

  pthread_t worker = xpthread_create (NULL, worker_thread, NULL);
  pthread_t setxid = xpthread_create (NULL, setxid_thread, NULL);

  /* Lower the pending-signal limit, and fill the realtime signal queue until
     it reaches RLIMIT_SIGPENDING.  */
  {
    struct rlimit rl;
    TEST_COMPARE (getrlimit (RLIMIT_SIGPENDING, &rl), 0);
    rl.rlim_cur = 64;
    TEST_COMPARE (setrlimit (RLIMIT_SIGPENDING, &rl), 0);
  }

  {
    const union sigval val = { .sival_int = 0 };
    while (sigqueue (getpid (), SIGRTMIN, val) == 0)
      continue;
    TEST_COMPARE (errno, EAGAIN);
  }

  xpthread_barrier_wait (&start_barrier);

  struct timespec ts;
  TEST_COMPARE (clock_gettime (CLOCK_REALTIME, &ts), 0);
  ts.tv_sec += BLOCK_WAIT_SECONDS;
  int r;
  while ((r = sem_timedwait (&setxid_done, &ts)) != 0 && errno == EINTR)
    /* Restart if interrupted by the SIGSETXID handler.  The deadline is
       absolute, so the wait window is not extended.  */;

  if (r == 0)
    /* The setxid completed while the queue was still full: the broadcast
       silently gave up instead of delivering SIGSETXID (bug 21108).  */
    FAIL_EXIT1 ("setxid returned while the signal queue was full");
  TEST_COMPARE (errno, ETIMEDOUT);

  /* Now drain the queue so the (correctly blocking) setxid can finish.  */
  while (1)
    {
      struct timespec zero = { 0, 0 };
      if (sigtimedwait (&set, NULL, &zero) < 0)
	{
	  if (errno == EINTR)
	    continue;
	  break;
	}
    }

  /* With the queue drained, the setxid must complete.  */
  while (sem_wait (&setxid_done) != 0)
    /* Restart if interrupted.  */;

  /* Release the worker and clean up.  */
  TEST_COMPARE (sem_post (&worker_exit), 0);
  xpthread_join (worker);
  xpthread_join (setxid);
  return 0;
}

#define TIMEOUT 10
#include <support/test-driver.c>
