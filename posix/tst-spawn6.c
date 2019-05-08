/* Tests for posix_spawn signal handling.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <spawn.h>
#include <sys/wait.h>

#include <support/check.h>
#include <support/xunistd.h>
#include <support/support.h>
#include <array_length.h>

/* Nonzero if the program gets called via `exec'.  */
static int restart;
#define CMDLINE_OPTIONS \
  { "restart", no_argument, &restart, 1 },

enum spawn_test_t
{
  SPAWN_SETSIGMASK,
  SPAWN_SETSIGDEF
};

static int signal_to_check[] =
{
  SIGHUP, SIGINT, SIGALRM, SIGUSR2
};

/* Called on process re-execution.  */
static int
handle_restart (enum spawn_test_t test)
{
  switch (test)
    {
    case SPAWN_SETSIGMASK:
      {
	sigset_t mask;
	sigprocmask (SIG_BLOCK, NULL, &mask);
	for (int i = 0; i < array_length (signal_to_check); i++)
	  if (sigismember (&mask, signal_to_check[i]) != 1)
	    exit (EXIT_FAILURE);
      }
      break;
    case SPAWN_SETSIGDEF:
      {
	for (int i = 0; i < array_length (signal_to_check); i++)
	  {
	    struct sigaction act;
	    if (sigaction (signal_to_check[i], NULL, &act) != 0)
	      exit (EXIT_FAILURE);
	    if (act.sa_handler != SIG_DFL)
	      exit (EXIT_FAILURE);
	  }
      }
      break;
    }

  return 0;
}

/* Common argument used for process re-execution.  */
static char *initial_spargv[5];
static size_t initial_spargv_size;

/* Re-execute the test process with both '--direct', '--restart', and the
   TEST (as integer value) as arguments.  */
static void
reexecute (enum spawn_test_t test, const posix_spawnattr_t *attrp)
{
  char *spargv[8];
  int i;

  for (i = 0; i < initial_spargv_size; i++)
    spargv[i] = initial_spargv[i];
  /* Three digits per byte plus null terminator.  */
  char teststr[3 * sizeof (test) + 1];
  snprintf (teststr, array_length (teststr), "%d", test);
  spargv[i++] = teststr;
  spargv[i] = NULL;
  TEST_VERIFY (i < 8);

  pid_t pid;
  int status;

  TEST_COMPARE (posix_spawn (&pid, spargv[0], NULL, attrp, spargv, environ),
		0);
  TEST_COMPARE (xwaitpid (pid, &status, 0), pid);
  TEST_VERIFY (WIFEXITED (status));
  TEST_VERIFY (!WIFSIGNALED (status));
  TEST_COMPARE (WEXITSTATUS (status), 0);
}

/* Test if POSIX_SPAWN_SETSIGMASK change the spawn process signal mask to
   the value blocked signals defined by SIGNAL_TO_CHECK signals.  */
static void
do_test_setsigmask (void)
{
  posix_spawnattr_t attr;
  /* posix_spawnattr_init does not fail.  */
  posix_spawnattr_init (&attr);

  {
    sigset_t mask;
    TEST_COMPARE (sigemptyset (&mask), 0);
    for (int i = 0; i < array_length (signal_to_check); i++)
      TEST_COMPARE (sigaddset (&mask, signal_to_check[i]), 0);
    TEST_COMPARE (posix_spawnattr_setsigmask (&attr, &mask), 0);
    TEST_COMPARE (posix_spawnattr_setflags (&attr, POSIX_SPAWN_SETSIGMASK), 0);
  }

  /* Change current mask to be different than the one asked for spawned
     process.  */
  {
    sigset_t empty_mask, current_mask;
    TEST_COMPARE (sigemptyset (&empty_mask), 0);
    TEST_COMPARE (sigprocmask (SIG_BLOCK, &empty_mask, &current_mask), 0);

    reexecute (SPAWN_SETSIGMASK, &attr);

    TEST_COMPARE (sigprocmask (SIG_SETMASK, &current_mask, NULL), 0);
  }
}

/* Test if POSIX_SPAWN_SETSIGDEF change the spawn process signal actions
   defined by SIGNAL_TO_CHECK signals to default actions.  */
static void
do_test_setsigdef (void)
{
  posix_spawnattr_t attr;
  /* posix_spawnattr_init does not fail.  */
  posix_spawnattr_init (&attr);

  {
    sigset_t mask;
    TEST_COMPARE (sigemptyset (&mask), 0);
    for (int i = 0; i < array_length (signal_to_check); i++)
      TEST_COMPARE (sigaddset (&mask, signal_to_check[i]), 0);
    TEST_COMPARE (posix_spawnattr_setsigdefault (&attr, &mask), 0);
    TEST_COMPARE (posix_spawnattr_setflags (&attr, POSIX_SPAWN_SETSIGDEF), 0);
  }

  /* Change current signal disposition to be different than the one asked for
     spawned process.  */
  struct sigaction default_act[array_length (signal_to_check)];
  {
    sigset_t empty_mask;
    TEST_COMPARE (sigemptyset (&empty_mask), 0);
    for (int i = 0; i < array_length (signal_to_check); i++)
      TEST_COMPARE (sigaction (signal_to_check[i],
			       &((struct sigaction) { .sa_handler = SIG_IGN,
						      .sa_mask = empty_mask,
						      .sa_flags = 0 }),
			       &default_act[i]),
		    0);
  }

  reexecute (SPAWN_SETSIGDEF, &attr);

  /* Restore signal dispositions.  */
  for (int i = 0; i < array_length (signal_to_check); i++)
    TEST_COMPARE (sigaction (signal_to_check[i], &default_act[i], NULL), 0);
}

static int
do_test (int argc, char *argv[])
{
  /* We must have one or four parameters left if called initially:
       + path for ld.so		optional
       + "--library-path"	optional
       + the library path	optional
       + the application name

     Plus one parameter to indicate which test to execute through
     re-execution.

     So for default usage without --enable-hardcoded-path-in-tests, it
     will be called initially with 5 arguments and later with 2.  For
     --enable-hardcoded-path-in-tests it will be called with 2 arguments
     regardless.  */

  if (argc != (restart ? 2 : 5) && argc != 2)
    FAIL_EXIT1 ("wrong number of arguments (%d)", argc);

  if (restart)
    return handle_restart (atoi (argv[1]));

  {
    int i;
    for (i = 0; i < (argc == 5 ? 4 : 1); i++)
      initial_spargv[i] = argv[i + 1];
    initial_spargv[i++] = (char *) "--direct";
    initial_spargv[i++] = (char *) "--restart";
    initial_spargv_size = i;
  }

  do_test_setsigmask ();
  do_test_setsigdef ();

  return 0;
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
