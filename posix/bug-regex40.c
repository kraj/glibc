/* Test that allocation failures during regexec are reported cleanly
   with a backreference pattern (BZ #34949).
   Copyright The GNU Toolchain Authors.
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

/* With a backreference in the pattern, check_matching returned the
   REG_ESPACE error code instead of -2 when an allocation failed in its
   initialization block, and re_search_internal used it as a match
   position.  This test runs regexec once for each allocation it
   performs, failing that allocation, in a subprocess so that a crash
   is reported as a test failure.  The subject is empty, so the group
   is never closed and the injected failures stay in the initialization
   block of check_matching.  */

#include <errno.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <support/check.h>
#include <support/namespace.h>
#include <support/support.h>

/* Data structure allocated via MAP_SHARED, so that writes from the
   subprocess are visible.  */
struct shared_data
{
  /* Number of tracked allocations performed so far.  */
  volatile unsigned int allocation_count;

  /* If this number is reached, one allocation fails.  */
  volatile unsigned int failing_allocation;
};

/* Allocation count in shared mapping.  */
static struct shared_data *shared;

/* Returns true if a failure should be injected for this allocation.  */
static bool
fail_this_allocation (void)
{
  if (shared != NULL)
    {
      unsigned int count = shared->allocation_count;
      shared->allocation_count = count + 1;
      return count == shared->failing_allocation;
    }
  else
    return false;
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

static regex_t preg;

/* No-op subprocess to verify that support_isolate_in_subprocess does
   not perform any heap allocations.  */
static void
no_op (void *ignored)
{
}

/* Run regexec in a subprocess.  The pattern does not match the empty
   subject, and regexec also reports an allocation failure as
   REG_NOMATCH.  */
static void
test_in_subprocess (void *ignored)
{
  TEST_COMPARE (regexec (&preg, "", 0, NULL, 0), REG_NOMATCH);
}

static int
do_test (void)
{
  TEST_COMPARE (regcomp (&preg, "\\(a\\)*\\1", 0), 0);

  shared = support_shared_allocate (sizeof (*shared));

  /* Disable fault injection.  */
  shared->failing_allocation = ~0U;

  support_isolate_in_subprocess (no_op, NULL);
  TEST_COMPARE (shared->allocation_count, 0);

  shared->allocation_count = 0;
  support_isolate_in_subprocess (test_in_subprocess, NULL);

  /* The number of allocations in the successful case, plus some
     slack.  Once the number of expected allocations is exceeded,
     injecting further failures does not make a difference.  */
  unsigned int maximum_allocation_count = shared->allocation_count;
  printf ("info: regexec performs %u allocations\n",
	  maximum_allocation_count);
  TEST_VERIFY (maximum_allocation_count > 0);
  maximum_allocation_count += 10;

  for (unsigned int inject_at = 0; inject_at <= maximum_allocation_count;
       ++inject_at)
    {
      shared->allocation_count = 0;
      shared->failing_allocation = inject_at;
      support_isolate_in_subprocess (test_in_subprocess, NULL);
    }

  support_shared_free (shared);
  shared = NULL;
  regfree (&preg);

  return 0;
}

#include <support/test-driver.c>
