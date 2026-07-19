/* Verify that constructors of independent DSOs loaded from different
   threads do not interleave.
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


/* Two worker threads concurrently dlopen two independent DSOs (no DT_NEEDED
   between them).  Each DSO's constructor appends a character to a shared
   buffer in the main executable several times.  The test then asserts that
   the two constructors did not interleave: the buffer must contain either
   "AAAAABBBBB" or "BBBBBAAAAA".

   This is a property of the current implementation, not of POSIX or the ELF
   specification: dl_load_lock is held across the entire _dl_open call,
   including constructor execution, so dlopen calls from different threads
   are fully serialized.  */

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <support/check.h>
#include <support/xdlfcn.h>
#include <support/xthread.h>

#include "tst-create4.h"

_Atomic int tst_create4_seq_idx = 0;
char tst_create4_seq_buf[TST_CREATE4_NSTEPS * 2 + 1];

static pthread_barrier_t start_barrier;

static void *
worker_a (void *unused)
{
  xpthread_barrier_wait (&start_barrier);
  void *h = xdlopen ("tst-create4mod-a.so", RTLD_NOW);
  xdlclose (h);
  return NULL;
}

static void *
worker_b (void *unused)
{
  xpthread_barrier_wait (&start_barrier);
  void *h = xdlopen ("tst-create4mod-b.so", RTLD_NOW);
  xdlclose (h);
  return NULL;
}

static int
do_test (void)
{
  xpthread_barrier_init (&start_barrier, NULL, 2);

  pthread_t ta = xpthread_create (0, worker_a, NULL);
  pthread_t tb = xpthread_create (0, worker_b, NULL);
  xpthread_join (ta);
  xpthread_join (tb);

  int len = atomic_load_explicit (&tst_create4_seq_idx,
				  memory_order_acquire);
  TEST_COMPARE (len, TST_CREATE4_NSTEPS * 2);
  tst_create4_seq_buf[len] = '\0';

  printf ("info: observed sequence: %s\n", tst_create4_seq_buf);

  if (strcmp (tst_create4_seq_buf, "AAAAABBBBB") != 0
      && strcmp (tst_create4_seq_buf, "BBBBBAAAAA") != 0)
    FAIL ("constructors of independent DSOs interleaved: \"%s\"",
	  tst_create4_seq_buf);

  return 0;
}

#include <support/test-driver.c>
