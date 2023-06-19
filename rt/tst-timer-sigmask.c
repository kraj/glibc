/* Check resulting signal mask from POSIX timer using SIGEV_THREAD.
   Copyright (C) 2020-2026 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

#include <support/check.h>
#include <support/test-driver.h>
#include <support/xthread.h>

#include <internal-signals.h>

static pthread_barrier_t barrier;

static void
thread_handler (union sigval sv)
{
  sigset_t ss;
  sigprocmask (SIG_BLOCK, NULL, &ss);
  if (test_verbose > 0)
    printf ("%s: blocked signal mask = { ", __func__);
  for (int sig = 1; sig < NSIG; sig++)
    {
      /* While the notification function runs, the SIGEV_THREAD helper blocks
	 all signals except SIGKILL, SIGSTOP, SIGSETXID, and SIGCANCEL (the
	 last, which aliases SIGTIMER, is unblocked around the notification
	 function so that it can be cancelled).  */
      if (sigismember (&ss, sig))
	TEST_VERIFY (sig != SIGKILL && sig != SIGSTOP && sig != SIGSETXID
		     && sig != SIGCANCEL);
      if (test_verbose && sigismember (&ss, sig))
	printf ("%d, ", sig);
    }
  if (test_verbose > 0)
    printf ("}\n");

  /* SIGCANCEL must be unblocked here so pthread_cancel is honored.  */
  TEST_VERIFY (!sigismember (&ss, SIGCANCEL));

  xpthread_barrier_wait (&barrier);
}

static int
do_test (void)
{
  struct sigevent sev = { };
  sev.sigev_notify = SIGEV_THREAD;
  sev.sigev_notify_function = &thread_handler;

  timer_t timerid;
  TEST_COMPARE (timer_create (CLOCK_REALTIME, &sev, &timerid), 0);

  xpthread_barrier_init (&barrier, NULL, 2);

  struct itimerspec trigger = { };
  trigger.it_value.tv_nsec = 1000000;
  TEST_COMPARE (timer_settime (timerid, 0, &trigger, NULL), 0);

  xpthread_barrier_wait (&barrier);

  return 0;
}

#include <support/test-driver.c>
