/* Test glob expansion of patterns with many components (BZ 34453).
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

#include <glob.h>
#include <stdlib.h>
#include <string.h>
#include <support/check.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/xunistd.h>
#include <sys/resource.h>

/* The stack the expansions below must fit in, whatever the limit of the
   system running the test is.  */
#define MAX_STACK_SIZE (8 * 1024 * 1024)

/* Number of components in the deep wildcard patterns.  */
#define DEEP 100000

/* Number of expressions in the deep brace patterns.  Every pending expansion
   holds its own copy of what remains of the pattern, so the peak memory use
   is quadratic in the count.  */
#define DEEP_BRACE 4000

/* A pattern with a nonexistent leading directory followed by DEEP wildcard
   components.  The leading component decides that nothing can match.  */
static char *
make_wildcard_pattern (void)
{
  static const char prefix[] = "__glibc_glob_missing__/";
  char *pattern = xmalloc (sizeof prefix + 2 * DEEP + 1);
  char *p = mempcpy (pattern, prefix, sizeof prefix - 1);

  for (size_t i = 0; i < DEEP; i++)
    p = mempcpy (p, "*/", 2);
  *p++ = 'x';
  *p = '\0';
  return pattern;
}

/* "{a}{a}...{a}x".  Brace expansion recursed once per brace expression.  */
static char *
make_brace_pattern (void)
{
  char *pattern = xmalloc (3 * DEEP_BRACE + 2);
  char *p = pattern;

  for (size_t i = 0; i < DEEP_BRACE; i++)
    p = mempcpy (p, "{a}", 3);
  *p++ = 'x';
  *p = '\0';
  return pattern;
}

/* Run in an empty temporary directory, so that neither the leading
   component of the wildcard pattern nor the "a" of the brace expansions
   exists.  */
static void
do_prepare (int argc, char *argv[])
{
  char *tmpdir = support_create_temp_directory ("tst-glob-bz34453-");
  xchdir (tmpdir);
  free (tmpdir);
}

static int
do_test (void)
{
  struct rlimit stack_limit = { MAX_STACK_SIZE, MAX_STACK_SIZE };
  TEST_VERIFY_EXIT (setrlimit (RLIMIT_STACK, &stack_limit) == 0);

  {
    glob_t g;
    char *pattern = make_wildcard_pattern ();

    TEST_COMPARE (glob (pattern, 0, NULL, &g), GLOB_NOMATCH);
    globfree (&g);

    /* With GLOB_NOCHECK the pattern itself is returned.  */
    TEST_COMPARE (glob (pattern, GLOB_NOCHECK, NULL, &g), 0);
    TEST_COMPARE (g.gl_pathc, 1);
    TEST_COMPARE_STRING (g.gl_pathv[0], pattern);
    globfree (&g);
    free (pattern);
  }

  {
    glob_t g;
    /* "a" does not exist, so no brace expansion matches.  */
    char *pattern = make_brace_pattern ();

    TEST_COMPARE (glob (pattern, GLOB_BRACE, NULL, &g), GLOB_NOMATCH);
    globfree (&g);
    free (pattern);
  }

  return 0;
}

#define PREPARE do_prepare
#include <support/test-driver.c>
