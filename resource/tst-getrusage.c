/* Test of the getrusage function.
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
#include <stdio.h>
#include <sys/resource.h>
#include <stdint.h>
#include <support/check.h>
#include <sys/types.h>
#include <support/xunistd.h>
#include <sys/wait.h>
#include <benchtests/bench-util.h>
#include <array_length.h>

static void
do_work (void)
{
  uint64_t sink = 0;
  for (uint64_t i = 0; i < UINT64_C (1000000); i++)
    {
      sink += i;
      DO_NOT_OPTIMIZE_OUT (sink);
    }
}

static void
get_child_elapsed_time (long *result)
{
  pid_t pid;
  pid = xfork ();

  if (pid > 0)
    {
      int status;
      xwaitpid (pid, &status, 0);
      TEST_VERIFY_EXIT (WIFEXITED (status));

      struct rusage rusage = { 0 };

      int ret_val = getrusage (RUSAGE_CHILDREN, &rusage);
      TEST_VERIFY_EXIT (ret_val == 0);
      long elapsed_time
	  = (rusage.ru_utime.tv_sec * 1000000L) + rusage.ru_utime.tv_usec;
      TEST_VERIFY_EXIT (elapsed_time >= 0);

      *result = elapsed_time;
    }
  else
    {
      /* Simulate some work. */
      do_work ();
      _exit (0);
    }
}

static void
test_getrusage_children (void)
{
  long elapsed_time_before;
  long elapsed_time_after;

  get_child_elapsed_time (&elapsed_time_before);
  get_child_elapsed_time (&elapsed_time_after);
  TEST_VERIFY_EXIT (elapsed_time_after >= elapsed_time_before);
}

static void
test_getrusage (int who)
{
  struct rusage before_usage = { 0 };
  struct rusage after_usage = { 0 };

  int ret_val = getrusage (who, &before_usage);
  TEST_VERIFY_EXIT (ret_val == 0);
  long elapsed_time_before = (before_usage.ru_utime.tv_sec * 1000000L)
			     + before_usage.ru_utime.tv_usec;
  TEST_VERIFY_EXIT (elapsed_time_before >= 0);

  /* Simulate some work */
  do_work ();

  int ret_val2 = getrusage (who, &after_usage);
  TEST_VERIFY_EXIT (ret_val2 == 0);

  long elapsed_time_after = (after_usage.ru_utime.tv_sec * 1000000L)
			    + after_usage.ru_utime.tv_usec;
  TEST_VERIFY_EXIT (elapsed_time_after >= elapsed_time_before);
}

static int
do_test (void)
{
  test_getrusage (RUSAGE_SELF);
#ifdef RUSAGE_THREAD
  test_getrusage (RUSAGE_THREAD);
#endif

  /* RUSAGE_CHILDREN test case with forking children */
  test_getrusage_children ();

  /* Invalid who value test case */
  struct rusage usage_struct = { 0 };
  int invalid_who = 999;
  int ret_val = getrusage (invalid_who, &usage_struct);
  TEST_VERIFY (ret_val == -1);

  return 0;
}

#include <support/test-driver.c>
