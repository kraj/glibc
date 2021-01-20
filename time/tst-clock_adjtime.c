/* Test for clock_adjtime
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <support/check.h>
#include <support/xtime.h>

static int
do_test (void)
{
  struct timespec tv_then, tv_now;
  struct timex delta;

  /* Check if altering target time is allowed.  */
  if (getenv (SETTIME_ENV_NAME) == NULL)
    FAIL_UNSUPPORTED ("clock_adjtime is executed only when "\
                      SETTIME_ENV_NAME" is set\n");

  tv_then = xclock_now (CLOCK_REALTIME);

  /* Setup time value to adjust - 1 ms. */
  delta.time.tv_sec = 0;
  delta.time.tv_usec = 1000;
  delta.modes = ADJ_SETOFFSET;

  int ret = clock_adjtime (CLOCK_REALTIME, &delta);
  if (ret == -1)
    FAIL_EXIT1 ("*** clock_adjtime failed: %m\n");

  tv_now = xclock_now (CLOCK_REALTIME);

  /* Check difference between timerfd_gettime calls.  */
  long long int diff = tv_now.tv_sec - tv_then.tv_sec;
  diff *= 1000000000;
  diff += tv_now.tv_nsec - tv_then.tv_nsec;
  /* The adjustment cannot be large than 10 ms.  */
  if (diff > 10000000)
    FAIL_EXIT1 ("clock_adjtime set wrong time!\n");

  return 0;
}

#include <support/test-driver.c>
