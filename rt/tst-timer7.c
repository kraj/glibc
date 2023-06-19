/* Check if thread local storage is reset on each SIGEV_THREAD trigger.
   Copyright (C) 2026 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <array_length.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include <support/check.h>

static sem_t sem;

static __thread int var1;
#define VAR2_LEN 32
static __thread char var2[] = { [0 ... VAR2_LEN] = 0xcc };

static const char var2_expected[] = { [0 ... VAR2_LEN] = 0xcc };

static void
on_timer (union sigval sv)
{
  TEST_COMPARE (var1, 0);
  TEST_COMPARE_BLOB (var2, array_length (var2),
		     var2_expected, array_length (var2_expected));

  var1 = 1;
  memset (var2, 0x00, array_length (var2));

  sem_post (&sem);
}

#define NITERS 10

static int
do_test (void)
{
  const struct itimerspec its =
    { .it_value    = { .tv_nsec = 10000000 /* 0.01s */ },
      .it_interval = { .tv_nsec = 10000000 /* 0.01s */ } };
  const struct itimerspec its_stop = { 0 };

  sem_init (&sem, 0, 0);

  timer_t timerid;
  struct sigevent ev =
    {
      .sigev_notify = SIGEV_THREAD,
      .sigev_notify_function = on_timer,
    };
  if (timer_create (CLOCK_REALTIME, &ev, &timerid) == -1)
    FAIL_EXIT1 ("timer_create: %m");

  if (timer_settime (timerid, 0, &its, NULL) == -1)
    FAIL_EXIT1 ("timer_settime: %m");

  for (int i = 0; i < NITERS; i++)
    {
      if (sem_wait (&sem) != 0)
	FAIL_EXIT1 ("sem_wait: %m");
    }

  /* Disarm before deleting to minimise the chance of an in-flight
     invocation racing with timer_delete.  */
  if (timer_settime (timerid, 0, &its_stop, NULL) == -1)
    FAIL_EXIT1 ("timer_settime: %m");

  if (timer_delete (timerid) == -1)
    FAIL_EXIT1 ("timer_delete: %m");

  sem_destroy (&sem);

  return 0;
}

#include <support/test-driver.c>
