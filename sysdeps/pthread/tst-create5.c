/* Verify that dlopen (NULL) from a worker thread does not deadlock.
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


/* An implementation that tracks per-object initialization state must
   mark the main executable as fully initialized, otherwise a later
   multi-threaded dlopen (NULL) /__RTLD_OPENEXEC call takes the
   already-loaded early-return path in _dl_open and can wait forever for a
   "pending" constructor.  */

#include <stdio.h>
#include <support/xdlfcn.h>
#include <support/xthread.h>

static void *
worker (void *arg)
{
  printf ("worker: dlopen(NULL)\n");
  void *h = xdlopen (NULL, RTLD_NOW);
  printf ("worker: dlopen(NULL) done\n");
  xdlclose (h);
  return NULL;
}

static int
do_test (void)
{
  pthread_t t = xpthread_create (0, worker, NULL);
  xpthread_join (t);

  printf ("main: worker finished\n");
  return 0;
}

#include <support/test-driver.c>
