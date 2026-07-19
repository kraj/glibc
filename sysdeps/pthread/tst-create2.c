/* Verify that a thread spawned by a dlopen constructor can register a
   TLS destructor without deadlocking (BZ 15686).
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


/* thread 1: dlopen -> ctor -> pthread_create (worker) -> pthread_join
   thread 2 (worker): __cxa_thread_atexit_impl -> lock (dl_load_lock)

   dl_load_lock is held by thread 1 across the constructor execution, so if
   __cxa_thread_atexit_impl acquires it the worker thread blocks forever and
   pthread_join in the constructor never returns.  */

#include <stdio.h>
#include <support/check.h>
#include <support/xdlfcn.h>

static int
do_test (void)
{
  printf ("main: dlopen tst-create2mod.so\n");
  void *h = xdlopen ("tst-create2mod.so", RTLD_NOW);
  printf ("main: dlopen done\n");

  /* The worker thread exited before the constructor's pthread_join
     returned, so its TLS destructor has already run.  */
  int *dtor_done = xdlsym (h, "tst_create2mod_dtor_done");
  TEST_COMPARE (*dtor_done, 1);

  xdlclose (h);
  printf ("main: dlclose done\n");

  /* The destructor already ran, so no reference is left on the
     module's l_tls_dtor_count and dlclose must have unloaded it.  */
  TEST_VERIFY (dlopen ("tst-create2mod.so", RTLD_NOW | RTLD_NOLOAD)
	       == NULL);

  return 0;
}

#include <support/test-driver.c>
