/* Basic glob tests.  It uses an extenal driver script (tst-glob.sh).
   Copyright (C) 1997-2017 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>

#include <support/check.h>

#define OPT_BRACE		'b'
#define OPT_NOCHECK		'c'
#define OPT_ONLYDIR		'd'
#define OPT_NOESCAPE		'e'
#define OPT_ERR			'E'
#define OPT_NOMAGIC		'g'
#define OPT_MARK		'm'
#define OPT_DOOFFS		'o'
#define OPT_PERIOD		'p'
#define OPT_QUOTES		'q'
#define OPT_NOSORT		's'
#define OPT_TILDE		't'
#define OPT_TILDE_CHECK		'T'

#define CMDLINE_OPTSTRING "bcdeEgmopqstT"

static int glob_flags = 0;
static int quotes = 1;

static void
cmdline_process_function (int c)
{
  switch (c)
    {
    case OPT_BRACE:
      glob_flags |= GLOB_BRACE;
      break;
    case OPT_NOCHECK:
      glob_flags |= GLOB_NOCHECK;
      break;
    case OPT_ONLYDIR:
      glob_flags |= GLOB_ONLYDIR;
      break;
    case OPT_NOESCAPE:
      glob_flags |= GLOB_NOESCAPE;
      break;
    case OPT_ERR:
      glob_flags |= GLOB_ERR;
      break;
    case OPT_NOMAGIC:
      glob_flags |= GLOB_NOMAGIC;
      break;
    case OPT_MARK:
      glob_flags |= GLOB_MARK;
      break;
    case OPT_DOOFFS:
      glob_flags |= GLOB_DOOFFS;
      break;
    case OPT_PERIOD:
      glob_flags |= GLOB_PERIOD;
      break;
    case OPT_QUOTES:
      quotes = 0;
      break;
    case OPT_NOSORT:
      glob_flags |= GLOB_NOSORT;
      break;
    case OPT_TILDE:
      glob_flags |= GLOB_TILDE;
      break;
    case OPT_TILDE_CHECK:
      glob_flags |= GLOB_TILDE_CHECK;
      break;
    }
}

#define CMDLINE_PROCESS	cmdline_process_function

static int
do_test_argv (int argc, char *argv[])
{
  int i, j;
  glob_t g;

  g.gl_offs = glob_flags & GLOB_DOOFFS ? 1 : 0;

  if (argc < 2)
    FAIL_EXIT1 ("invalid arguments (expecting path for glob)");
  if (chdir (argv[1]) != 0)
    FAIL_EXIT1 ("chmod (%s): %m", argv[1]);

  /* Do a glob on each argument.  */
  for (j = 2; j < argc; j++)
    {
      i = glob (argv[j], glob_flags, NULL, &g);
      if (i != 0)
	break;
      glob_flags |= GLOB_APPEND;
    }

  /* Was there an error? */
  if (i == GLOB_NOSPACE)
    puts ("GLOB_NOSPACE");
  else if (i == GLOB_ABORTED)
    puts ("GLOB_ABORTED");
  else if (i == GLOB_NOMATCH)
    puts ("GLOB_NOMATCH");

  /* If we set an offset, fill in the first field.
     (Unless glob() has filled it in already - which is an error) */
  if ((glob_flags & GLOB_DOOFFS) && g.gl_pathv[0] == NULL)
    g.gl_pathv[0] = (char *) "abc";

  /* Print out the names.  Unless otherwise specified, quote them.  */
  if (g.gl_pathv)
    {
      for (i = 0; i < g.gl_offs + g.gl_pathc; ++i)
	printf ("%s%s%s\n", quotes ? "`" : "",
		g.gl_pathv[i] ? g.gl_pathv[i] : "(null)", quotes ? "'" : "");
    }

  globfree (&g);

  return 0;
}

#define TEST_FUNCTION_ARGV do_test_argv
#include <support/test-driver.c>
