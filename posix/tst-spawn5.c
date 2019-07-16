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
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

#include <support/check.h>
#include <support/xunistd.h>
#include <support/support.h>
#include <array_length.h>

/* Nonzero if the program gets called via `exec'.  */
static int restart;
#define CMDLINE_OPTIONS \
  { "restart", no_argument, &restart, 1 },

/* Called on process re-execution.  */
static int
handle_restart (int from)
{
  DIR *fds = opendir ("/proc/self/fd");
  if (fds == NULL)
    FAIL_EXIT1 ("opendir (\"/proc/self/fd\"): %m");

  while (true)
    {
      errno = 0;
      struct dirent64 *e = readdir64 (fds);
      if (e == NULL)
        {
          if (errno != 0)
            FAIL_EXIT1 ("readdir: %m");
          break;
        }

      if (e->d_name[0] == '.')
        continue;

      char *endptr;
      long int fd = strtol (e->d_name, &endptr, 10);
      if (*endptr != '\0' || fd < 0 || fd > INT_MAX)
        FAIL_EXIT1 ("readdir: invalid file descriptor name: /proc/self/fd/%s",
                    e->d_name);

      /* Skip the descriptor which is used to enumerate the
         descriptors.  */
      if (fd == dirfd (fds))
        continue;

      struct stat64 st;
      if (fstat64 (fd, &st) != 0)
        FAIL_EXIT1 ("readdir: fstat64 (%ld) failed: %m", fd);

      if (fd >= from)
	FAIL_EXIT1 ("error: fd (%ld) greater than from (%d)", fd, from);
    }

  closedir (fds);

  return 0;
}

/* Common argument used for process re-execution.  */
static char *initial_spargv[5];
static size_t initial_spargv_size;

/* Re-execute the test process with both '--direct', '--restart', and the
   TEST (as integer value) as arguments.  */
static void
reexecute (int fd, const posix_spawn_file_actions_t *fa)
{
  char *spargv[8];
  int i;

  for (i = 0; i < initial_spargv_size; i++)
    spargv[i] = initial_spargv[i];
  /* Three digits per byte plus null terminator.  */
  char teststr[3 * sizeof (fd) + 1];
  snprintf (teststr, array_length (teststr), "%d", fd);
  spargv[i++] = teststr;
  spargv[i] = NULL;
  TEST_VERIFY (i < 8);

  pid_t pid;
  int status;

  TEST_COMPARE (posix_spawn (&pid, spargv[0], fa, NULL, spargv, environ),
		0);
  TEST_COMPARE (xwaitpid (pid, &status, 0), pid);
  TEST_VERIFY (WIFEXITED (status));
  TEST_VERIFY (!WIFSIGNALED (status));
  TEST_COMPARE (WEXITSTATUS (status), 0);
}

static void
do_test_closefrom (int num_fd_to_open)
{
  int *fds = xmalloc (num_fd_to_open * sizeof (int));
  for (int i = 0; i < num_fd_to_open; i++)
    fds[i] = xopen ("/dev/null", O_WRONLY, 0);

  posix_spawn_file_actions_t fa;
  /* posix_spawn_file_actions_init does not fail.  */
  posix_spawn_file_actions_init (&fa);

  {
    int ret = posix_spawn_file_actions_addclosefrom_np (&fa, fds[0]);
    if (ret == -1)
      {
	if (errno == ENOSYS)
	  /* Hurd currently does not support closefrom fileaction.  */
	  FAIL_UNSUPPORTED ("posix_spawn_file_actions_addclosefrom_np unsupported");
        else
	  FAIL_EXIT1 ("posix_spawn_file_actions_addclosefrom_np failed");
      }
  }

  /* Default check, all file descriptor from [fd[0], fd[1]) are opened.  */
  reexecute (fds[0], &fa);

  /* Add a gap in the range.  */
  xclose (fds[num_fd_to_open/2]);
  xclose (fds[num_fd_to_open/2 + 1]);
  reexecute (fds[0], &fa);

  /* Add another gap, at the beginning.  */
  xclose (fds[0]);
  xclose (fds[1]);
  reexecute (fds[0], &fa);

  /* Add another gap, now at the end.  */
  xclose (fds[num_fd_to_open-1]);
  xclose (fds[num_fd_to_open-2]);
  reexecute (fds[0], &fa);

  /* Open some more files, filling the gaps.  */
  for (int i = 0; i < 6; i++)
    xopen ("/dev/null", O_WRONLY, 0);
  reexecute (fds[0], &fa);

  /* Open some more, but with O_CLOEXEC.  */
  for (int i = 0; i < num_fd_to_open/2; i++)
    xopen ("/dev/null", O_WRONLY | O_CLOEXEC, 0);

  free (fds);
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

  /* Respawn using the same arguments.  */
  for (initial_spargv_size = 0;
       initial_spargv_size < (argc == 5 ? 4 : 1);
       initial_spargv_size++)
    initial_spargv[initial_spargv_size] = argv[initial_spargv_size + 1];
  initial_spargv[initial_spargv_size++] = (char *) "--direct";
  initial_spargv[initial_spargv_size++] = (char *) "--restart";

  do_test_closefrom (10);
  do_test_closefrom (100);

  return 0;
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
