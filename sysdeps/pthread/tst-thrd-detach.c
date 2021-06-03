/* C11 threads thread detach tests.
   Copyright (C) 2018-2021 Free Software Foundation, Inc.
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

#include <threads.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <support/check.h>

static int
detach_thrd (void *arg)
{
  thrd_sleep (&(struct timespec) { .tv_sec = INT_MAX }, NULL);

  thrd_exit (thrd_success);
}

static int
do_test (void)
{
  thrd_t id;

  TEST_COMPARE (thrd_create (&id, detach_thrd, NULL), thrd_success);

  TEST_COMPARE (thrd_detach (id), thrd_success);

  TEST_COMPARE (thrd_join (id, NULL), thrd_error);

  return 0;
}

#include <support/test-driver.c>
