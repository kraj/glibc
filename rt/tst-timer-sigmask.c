/* Check resulting signal mask from POSIX timer using SIGEV_THREAD.
   Copyright (C) 2020-2023 Free Software Foundation, Inc.
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
__attribute_used__
cl (void *arg)
{
  xpthread_barrier_wait (&barrier);
}

static void
thread_handler (union sigval sv)
{
  /* POSIX timers threads created to handle SIGEV_THREAD block all
     signals except SIGKILL, SIGSTOP, and SIGSETXID.  */
  sigset_t expected_set = {
    .__val = {[0 ...  __NSIG_WORDS-1 ] = -1 }
  };
  __sigdelset (&expected_set, SIGSETXID);
  __sigdelset (&expected_set, SIGKILL);
  __sigdelset (&expected_set, SIGSTOP);

  sigset_t ss;
  sigprocmask (SIG_SETMASK, NULL, &ss);

  TEST_COMPARE_BLOB (&ss.__val, __NSIG_BYTES, &expected_set, __NSIG_BYTES);

  /* Unblock some signals, the signal mask should be reset by the timer_create
     helper thread.  */
  {
    sigset_t toset;
    sigemptyset (&toset);
    sigaddset (&toset, SIGHUP);
    sigaddset (&toset, SIGILL);
    sigprocmask (SIG_UNBLOCK, &toset, NULL);
  }

  pthread_cleanup_push (cl, NULL);
  pthread_exit (NULL);
  pthread_cleanup_pop (0);
}

static int
do_test (void)
{
  struct sigevent sev = { 0 };
  sev.sigev_notify = SIGEV_THREAD;
  sev.sigev_notify_function = &thread_handler;

  timer_t timerid;
  TEST_COMPARE (timer_create (CLOCK_REALTIME, &sev, &timerid), 0);

  xpthread_barrier_init (&barrier, NULL, 2);

  struct itimerspec trigger = { { 0, 1000000 }, { 0, 1000000 } };
  TEST_COMPARE (timer_settime (timerid, 0, &trigger, NULL), 0);

  enum { TIMERS_TO_CHECK = 2 };
  for (int i = 0; i < TIMERS_TO_CHECK; i++)
    xpthread_barrier_wait (&barrier);

  return 0;
}

#include <support/test-driver.c>
