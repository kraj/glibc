/* Test dlopen handling of OOM with injected allocation failures (bug 34769).
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

#include <dlfcn.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <support/check.h>
#include <support/namespace.h>
#include <support/support.h>
#include <support/xdlfcn.h>

struct shared_data
{
  /* Number of tracked allocations performed so far.  */
  unsigned int allocation_count;

  /* If this number is reached, all subsequent allocations fail.  */
  unsigned int failing_allocation;

  bool seen_failure;
  bool open_succeeded;

  char error[1024];
};

static struct shared_data *shared;

/* Returns true if a failure should be injected for this allocation.  */
static bool
fail_this_allocation (void)
{
  if (shared == NULL)
    return false;

  unsigned int count = shared->allocation_count;
  shared->allocation_count = count + 1;
  bool will_fail = (count >= shared->failing_allocation);
  shared->seen_failure = shared->seen_failure || will_fail;
  return will_fail;
}

/* Failure-injecting wrappers for allocation functions used by glibc.  */

void *
malloc (size_t size)
{
  if (fail_this_allocation ())
    {
      errno = ENOMEM;
      return NULL;
    }
  extern __typeof (malloc) __libc_malloc;
  return __libc_malloc (size);
}

void *
calloc (size_t a, size_t b)
{
  if (fail_this_allocation ())
    {
      errno = ENOMEM;
      return NULL;
    }
  extern __typeof (calloc) __libc_calloc;
  return __libc_calloc (a, b);
}

void *
realloc (void *ptr, size_t size)
{
  if (fail_this_allocation ())
    {
      errno = ENOMEM;
      return NULL;
    }
  extern __typeof (realloc) __libc_realloc;
  return __libc_realloc (ptr, size);
}

/* dlopen a module with RTLD_NODELETE while injecting allocation
   failures, and verify that a failed dlopen does not leave the
   NODELETE module loaded.

   Each attempt runs in a fresh subprocess because a successful
   NODELETE dlopen cannot be undone, and because some allocation
   failures are tolerated (dlopen still succeeds), so the sweep in
   do_test continues until dlopen succeeds without any injected
   failure.  */
static void
test_in_subprocess (void *unused)
{
  static const char mod[] = "tst-dl_find_object-mod1.so";
  void *mod1 = dlopen (mod, RTLD_LAZY | RTLD_NODELETE);
  shared->failing_allocation = ~0U;
  if (mod1 != NULL)
    {
      xdlsym (mod1, "mod1_data");
      shared->open_succeeded = true;
      return;
    }

  char *error = dlerror ();
  TEST_VERIFY (error != NULL);
  snprintf (shared->error, sizeof (shared->error), "%s", error);

  /* Now check nothing is left over from this failing dlopen.  */
  TEST_VERIFY (dlopen (mod, RTLD_LAZY | RTLD_NOLOAD) == NULL);
}

static int
do_test (void)
{
  static const int max_fails = 1000;
  int i = 0;
  shared = support_shared_allocate (sizeof (*shared));
  for (; i < max_fails; i++)
    {
      shared->allocation_count = 0;
      shared->failing_allocation = i;
      shared->seen_failure = false;
      shared->open_succeeded = false;

      support_isolate_in_subprocess (test_in_subprocess, NULL);
      if (shared->open_succeeded && !shared->seen_failure)
	break;
    }

  if (i == max_fails)
    FAIL_EXIT1 ("dlopen never succeeded within %d allocations\n"
		"last error: %s\n"
		"open_succeeded = %d, seen_failure = %d",
		max_fails, shared->error,
		(int) shared->open_succeeded, (int) shared->seen_failure);

  printf ("info: dlopen succeeded with %d allocations\n", i);
  return 0;
}

#include <support/test-driver.c>
