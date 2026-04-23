/* Test for pthread_mutex_lock deadlock behavior.
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

#include <array_length.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/support.h>
#include <support/test-driver.h>
#include <support/xthread.h>

#define ASSUME_DEADLOCK_AFTER_SECONDS 3

struct which_mutex
{
  int type;
  bool prio_inherit;
  bool robust;
};

struct task_context
{
  pthread_mutex_t *first, *second;
  pthread_barrier_t *barrier;
};

static bool
should_detect_deadlock (const struct which_mutex *const that)
{
  return that->prio_inherit && that->type == PTHREAD_MUTEX_ERRORCHECK;
}

static void *
thread_function (void *const arg)
{
  const struct task_context *ctx = arg;
  intptr_t ret = 0;
  xpthread_mutex_lock (ctx->first);
  xpthread_barrier_wait (ctx->barrier);
  ret = pthread_mutex_lock (ctx->second);
  xpthread_mutex_unlock (ctx->first);
  if (ret == 0)
    xpthread_mutex_unlock (ctx->second);
  return (void *) ret;
}

static void
prepare_mutex (pthread_mutex_t *const mutex,
               const struct which_mutex *const that)
{
  pthread_mutexattr_t attr;
  xpthread_mutexattr_init (&attr);
  xpthread_mutexattr_settype (&attr, that->type);
  if (that->robust)
    xpthread_mutexattr_setrobust (&attr, PTHREAD_MUTEX_ROBUST);
  if (that->prio_inherit)
    xpthread_mutexattr_setprotocol (&attr, PTHREAD_PRIO_INHERIT);
  xpthread_mutex_init (mutex, &attr);
  xpthread_mutexattr_destroy (&attr);
}

static void
do_test_single (void *const ctx)
{
  pthread_mutex_t m1, m2;
  pthread_barrier_t barrier;
  const struct which_mutex *const that = ctx;
  const bool graceful = should_detect_deadlock (that);
  struct task_context ctx1
      = { .first = &m1, .second = &m2, .barrier = &barrier };
  struct task_context ctx2
      = { .first = &m2, .second = &m1, .barrier = &barrier };
  xpthread_barrier_init (&barrier, NULL, 2);
  prepare_mutex (&m1, that);
  prepare_mutex (&m2, that);
  const pthread_t t1 = xpthread_create (NULL, thread_function, &ctx1);
  const pthread_t t2 = xpthread_create (NULL, thread_function, &ctx2);
  if (!graceful)
    delayed_exit (ASSUME_DEADLOCK_AFTER_SECONDS);
  const int ret1 = (intptr_t) xpthread_join (t1);
  const int ret2 = (intptr_t) xpthread_join (t2);
  xpthread_mutex_destroy (&m1);
  xpthread_mutex_destroy (&m2);
  xpthread_barrier_destroy (&barrier);
  TEST_VERIFY (graceful);
  TEST_VERIFY (ret1 == 0 || ret1 == EDEADLK);
  TEST_VERIFY (ret2 == 0 || ret2 == EDEADLK);
  TEST_VERIFY (ret1 == EDEADLK || ret2 == EDEADLK);
}

static int
do_test (void)
{
  const int mutex_types[] = {
    PTHREAD_MUTEX_TIMED_NP,
    PTHREAD_MUTEX_RECURSIVE_NP,
    PTHREAD_MUTEX_ERRORCHECK_NP,
    PTHREAD_MUTEX_ADAPTIVE_NP,
  };
  for (size_t i = 0; i < array_length (mutex_types); ++i)
    {
      for (int pi = 0; pi < 2; ++pi)
        {
          for (int rb = 0; rb < 2; ++rb)
            {
              struct which_mutex that = {
                .type = mutex_types[i],
                .prio_inherit = pi,
                .robust = rb,
              };
              const char *const description
                  = xasprintf ("type = %d, prio_inherit = %d, robust = %d",
                               that.type, that.robust, that.prio_inherit);
              struct support_capture_subprocess capture
                  = support_capture_subprocess (do_test_single, &that);
              support_capture_subprocess_check (&capture, description, 0,
                                                sc_allow_none);
              support_capture_subprocess_free (&capture);
            }
        }
    }
  return 0;
}

#define TIMEOUT 60
#include <support/test-driver.c>
