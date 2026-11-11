/* Test regcomp parsing with injected allocation failures (bug 33185).
   Copyright (C) 2025-2026 Free Software Foundation, Inc.
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

/* This test invokes regcomp multiple times, failing one memory
   allocation in each call.  The function call should fail with
   REG_ESPACE (or succeed if it can recover from the allocation
   failure).  Previously, there were double-free bugs on the partial
   allocation-failure paths of parse_bracket_exp (bug 33185),
   re_dfa_add_node and build_range_exp (bugs 31935 and 34991).

   To detect these deterministically, the allocator is interposed and
   every pointer it hands out is tracked while a regcomp call is in
   progress.  */

#include <errno.h>
#include <locale.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <support/check.h>
#include <support/namespace.h>
#include <support/support.h>
#include <support/xdlfcn.h>

/* Data structure allocated via MAP_SHARED, so that writes from the
   subprocess are visible.  */
struct shared_data
{
  /* Number of tracked allocations performed so far.  */
  volatile unsigned int allocation_count;

  /* If this number is reached, one allocation fails.  */
  volatile unsigned int failing_allocation;

  /* Incremented by the free interposer when a double free or a free of
     a stale pointer is detected during a tracked call.  */
  volatile unsigned int corruption;

  /* Set nonzero if the tracking registry overflows.  */
  volatile unsigned int overflow;
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

/* While TRACKING is set, every pointer returned by the allocator is recorded,
   and free is validated against the recorded set.  The registry itself never
   allocates.  */
enum { HEAP_MAX = 8192 };
static void *heap_ptr[HEAP_MAX];
static bool heap_live[HEAP_MAX];
static size_t heap_count;
static bool tracking;

static void *(*real_malloc) (size_t);
static void *(*real_calloc) (size_t, size_t);
static void *(*real_realloc) (void *, size_t);
static void (*real_free) (void *);

static void __attribute__ ((constructor))
resolve_allocator (void)
{
  real_malloc = xdlsym (RTLD_NEXT, "malloc");
  real_calloc = xdlsym (RTLD_NEXT, "calloc");
  real_realloc = xdlsym (RTLD_NEXT, "realloc");
  real_free = xdlsym (RTLD_NEXT, "free");
}

static void
heap_track_reset (void)
{
  heap_count = 0;
}

static void
heap_record_alloc (void *p)
{
  if (p == NULL)
    return;
  for (size_t i = 0; i < heap_count; ++i)
    if (heap_ptr[i] == p)
      {
	heap_live[i] = true;
	return;
      }
  if (heap_count < HEAP_MAX)
    {
      heap_ptr[heap_count] = p;
      heap_live[heap_count] = true;
      ++heap_count;
    }
  else if (shared != NULL)
    shared->overflow = shared->overflow + 1;
}

/* Result of validating a free against the tracking registry.  */
enum heap_free_status
{
  HEAP_FREE_OK,		/* P was live and is now marked released.  */
  HEAP_FREE_DOUBLE,	/* P was already released (double or stale free).  */
  HEAP_FREE_UNTRACKED,	/* P is not tracked (allocated before tracking).  */
};

/* Mark P as released and report whether the free is valid.  */
static enum heap_free_status
heap_record_free (void *p)
{
  for (size_t i = 0; i < heap_count; ++i)
    if (heap_ptr[i] == p)
      {
	if (!heap_live[i])
	  return HEAP_FREE_DOUBLE;
	heap_live[i] = false;
	return HEAP_FREE_OK;
      }
  return HEAP_FREE_UNTRACKED;
}

/* Failure-injecting, tracking wrappers for the allocation functions
   used by glibc.  */

void *
malloc (size_t size)
{
  if (fail_this_allocation ())
    {
      errno = ENOMEM;
      return NULL;
    }
  void *p = real_malloc (size);
  if (tracking)
    heap_record_alloc (p);
  return p;
}

void *
calloc (size_t a, size_t b)
{
  if (fail_this_allocation ())
    {
      errno = ENOMEM;
      return NULL;
    }
  void *p = real_calloc (a, b);
  if (tracking)
    heap_record_alloc (p);
  return p;
}

void *
realloc (void *ptr, size_t size)
{
  if (fail_this_allocation ())
    {
      errno = ENOMEM;
      return NULL;
    }
  void *p = real_realloc (ptr, size);
  if (tracking && p != NULL)
    {
      if (ptr != NULL && p != ptr)
	heap_record_free (ptr);
      heap_record_alloc (p);
    }
  return p;
}

void
free (void *ptr)
{
  if (ptr == NULL)
    return;
  if (tracking)
    {
      if (heap_record_free (ptr) == HEAP_FREE_DOUBLE)
	{
	  /* Record it and drop the free so the heap is not corrupted
	     further.  */
	  if (shared != NULL)
	    shared->corruption = shared->corruption + 1;
	  return;
	}
    }
  real_free (ptr);
}

/* No-op subprocess to verify that support_isolate_in_subprocess does
   not perform any heap allocations.  */
static void
no_op (void *ignored)
{
}

/* regcomp flags for the pattern under test.  */
static int test_cflags;

static int
compile_tracked (const char *regexp)
{
  regex_t reg;
  heap_track_reset ();
  shared->corruption = 0;
  shared->overflow = 0;
  tracking = true;
  int ret = regcomp (&reg, regexp, test_cflags);
  tracking = false;
  if (ret == 0)
    regfree (&reg);
  TEST_COMPARE (shared->corruption, 0);
  /* A registry overflow means tracking was incomplete.  */
  TEST_COMPARE (shared->overflow, 0);
  return ret;
}

/* Perform a regcomp call in a subprocess.  Used to count its
   allocations.  */
static void
initialize (void *regexp1)
{
  const char *regexp = regexp1;

  shared->allocation_count = 0;

  TEST_COMPARE (compile_tracked (regexp), 0);
}

/* Perform regcomp in a subprocess with fault injection.  */
static void
test_in_subprocess (void *regexp1)
{
  const char *regexp = regexp1;
  unsigned int inject_at = shared->failing_allocation;

  int ret = compile_tracked (regexp);

  if (ret != 0)
    {
      TEST_COMPARE (ret, REG_ESPACE);
      printf ("info: allocation %u failure results in return value %d,"
              " error %s (%d)\n",
              inject_at, ret, strerrorname_np (errno), errno);
    }
}

static void
run_pattern (const char *regexp, int cflags, const char *locale)
{
  test_cflags = cflags;

  /* Disable fault injection before switching locale, setlocale must not
     see an injected failure.  */
  shared->failing_allocation = ~0U;
  xsetlocale (LC_ALL, locale == NULL ? "C" : locale);

  /* Count the allocations of a successful call.  */
  support_isolate_in_subprocess (initialize, (void *) regexp);

  /* The number of allocations in the successful case, plus some
     slack.  Once the number of expected allocations is exceeded,
     injecting further failures does not make a difference.  */
  unsigned int maximum_allocation_count = shared->allocation_count;
  printf ("info: pattern \"%s\" (locale %s) performs %u allocations\n",
	  regexp, locale == NULL ? "C" : locale, maximum_allocation_count);
  maximum_allocation_count += 10;

  for (unsigned int inject_at = 0; inject_at <= maximum_allocation_count;
       ++inject_at)
    {
      shared->allocation_count = 0;
      shared->failing_allocation = inject_at;
      support_isolate_in_subprocess (test_in_subprocess, (void *) regexp);
    }
}

static int
do_test (void)
{
  shared = support_shared_allocate (sizeof (*shared));

  /* Disable fault injection.  */
  shared->failing_allocation = ~0U;

  support_isolate_in_subprocess (no_op, NULL);
  TEST_COMPARE (shared->allocation_count, 0);

  /* Bracket expression parsing (parse_bracket_exp, bug 33185).  */
  run_pattern ("[:alpha:]", 0, NULL);

  /* NFA node array growth in re_dfa_add_node (bug 34991).  The initial
     nodes_alloc is the pattern length + 1; the interval duplicates the
     'a' node 64 times, so the node arrays are grown several times.  */
  run_pattern ("a{64}", REG_EXTENDED, NULL);

  /* Range array growth in build_range_exp (bugs 31935 and 34991).  A
     multibyte locale stores the ranges in the mbcset arrays, and
     multiple ranges force the growth realloc.  */
  run_pattern ("[a-bc-de-fg-hi-jk-lm-no-pq-rs-t]", 0, "en_US.UTF-8");

  support_shared_free (shared);

  return 0;
}

#include <support/test-driver.c>
