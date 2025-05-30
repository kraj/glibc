/* Test for settimeofday
   Copyright (C) 2021-2025 Free Software Foundation, Inc.
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
#include <support/check.h>
#include <support/xtime.h>

#define TIMESPEC_SEC_Y2038_OV 0x7FFFFFFF
#define FUTURE_TIME (TIMESPEC_SEC_Y2038_OV - 10)

static int
do_test (void)
{
  const struct timeval tv = { FUTURE_TIME, 0};
  struct timespec tv_future, tv_now;

  /* Check if altering target time is allowed.  */
  if (getenv (SETTIME_ENV_NAME) == NULL)
    FAIL_UNSUPPORTED ("settimeofday is executed only when "\
                      SETTIME_ENV_NAME" is set\n");

  tv_now = xclock_now (CLOCK_REALTIME);
  int ret = settimeofday (&tv, NULL);
  if (ret == -1)
    FAIL_EXIT1 ("settimeofday failed: %m\n");

  tv_future = xclock_now (CLOCK_REALTIME);

  /* Restore old time value on target machine.  */
  xclock_settime (CLOCK_REALTIME, (const struct timespec*) &tv_now);

  if (tv_future.tv_sec < tv.tv_sec)
    FAIL_EXIT1 ("settimeofday set wrong time!\n");

  return 0;
}

#include <support/test-driver.c>
