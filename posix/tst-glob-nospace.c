/* Test glob with injected allocation failures (BZ 34993).
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

#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <support/check.h>
#include <support/namespace.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/xunistd.h>

struct shared_data
{
  /* Number of malloc calls performed so far.  */
  volatile unsigned int allocation_count;

  /* If this number is reached, one allocation fails.  */
  volatile unsigned int failing_allocation;
};

static struct shared_data *shared;

void *
malloc (size_t size)
{
  if (shared != NULL)
    {
      unsigned int count = shared->allocation_count;
      shared->allocation_count = count + 1;
      if (count == shared->failing_allocation)
	{
	  errno = ENOMEM;
	  return NULL;
	}
    }
  extern __typeof (malloc) __libc_malloc;
  return __libc_malloc (size);
}

/* The pattern, which do_prepare creates.  */
static char *pattern;

static void
run_glob (void *ignored)
{
  glob_t g;
  int ret = glob (pattern, 0, NULL, &g);
  if (shared->failing_allocation == ~0U)
    {
      TEST_COMPARE (ret, 0);
      TEST_COMPARE (g.gl_pathc, 4);
    }
  else
    /* glob skips a directory that opendir fails to allocate memory for,
       as for any other error opening it.  */
    TEST_VERIFY (ret == 0 || ret == GLOB_NOSPACE || ret == GLOB_NOMATCH);
  globfree (&g);
}

static void
do_prepare (int argc, char *argv[])
{
  /* Two directories with two files each.  */
  char *dir = support_create_temp_directory ("tst-glob-nospace-");
  for (char d = 'a'; d <= 'b'; d++)
    {
      char *sub = xasprintf ("%s/%c", dir, d);
      xmkdir (sub, 0700);
      add_temp_file (sub);
      for (char f = 'a'; f <= 'b'; f++)
	{
	  char *file = xasprintf ("%s/%c", sub, f);
	  xclose (xopen (file, O_WRONLY | O_CREAT, 0600));
	  add_temp_file (file);
	  free (file);
	}
      free (sub);
    }
  pattern = xasprintf ("%s/*/*", dir);
  free (dir);
}

static int
do_test (void)
{
  shared = support_shared_allocate (sizeof (*shared));
  shared->failing_allocation = ~0U;

  shared->allocation_count = 0;
  support_isolate_in_subprocess (run_glob, NULL);
  unsigned int count = shared->allocation_count;
  printf ("info: successful call performs %u allocations\n", count);

  for (unsigned int i = 0; i < count; i++)
    {
      shared->allocation_count = 0;
      shared->failing_allocation = i;
      support_isolate_in_subprocess (run_glob, NULL);
    }
  shared->failing_allocation = ~0U;

  support_shared_free (shared);
  return 0;
}

#define PREPARE do_prepare
#include <support/test-driver.c>
