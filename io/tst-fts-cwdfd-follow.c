/* Check fts_set FTS_FOLLOW on directories in an FTS_CWDFD walk (BZ 34781).
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

#include <array_length.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <support/check.h>
#include <support/descriptors.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/xunistd.h>

static char *root;

/* Create the tree:

     root/a/file
     root/b/file
     root/c/d/e/f/g/h/file
     root/z/file

   There are no symbolic links and the walk follows plain directories.  The
   nested directories under root/c are deep enough to exhaust the descriptor
   ring used by FTS_CWDFD, so the return from root/c can not rely on it.  */
static void
do_prepare (int argc, char **argv)
{
  char *tempdir = support_create_temp_directory ("tst-fts-cwdfd-follow-");
  root = xasprintf ("%s/root", tempdir);
  free (tempdir);
  xmkdir (root, 0700);
  add_temp_file (root);

  static const char *const dirs[] =
    {
      "a",
      "b",
      "c",
      "c/d",
      "c/d/e",
      "c/d/e/f",
      "c/d/e/f/g",
      "c/d/e/f/g/h",
      "z"
    };
  for (size_t i = 0; i < array_length (dirs); i++)
    {
      char *path = xasprintf ("%s/%s", root, dirs[i]);
      xmkdir (path, 0700);
      add_temp_file (path);
      free (path);
    }

  static const char *const files[] =
    {
      "a/file",
      "b/file",
      "c/d/e/f/g/h/file",
      "z/file"
    };
  for (size_t i = 0; i < array_length (files); i++)
    {
      char *path = xasprintf ("%s/%s", root, files[i]);
      xclose (xopen (path, O_WRONLY | O_CREAT | O_EXCL, 0600));
      add_temp_file (path);
      free (path);
    }
}
#define PREPARE do_prepare

static int
compare_name (const FTSENT **a, const FTSENT **b)
{
  return strcmp ((*a)->fts_name, (*b)->fts_name);
}

static const char *const expected[] =
  {
    "a",
    "a/file",
    "b",
    "b/file",
    "c",
    "c/d",
    "c/d/e",
    "c/d/e/f",
    "c/d/e/f/g",
    "c/d/e/f/g/h",
    "c/d/e/f/g/h/file",
    "z",
    "z/file"
  };

static void
check_walk (int options, bool sort)
{
  printf ("info: options=%#x sort=%d\n", options, sort);

  struct support_descriptors *descriptors = support_descriptors_list ();

  char *paths[] = { root, NULL };
  FTS *fts = fts_open (paths, options, sort ? compare_name : NULL);
  TEST_VERIFY_EXIT (fts != NULL);

  bool seen[array_length (expected)] = { false };
  /* Descriptors that the application opens during the walk.  */
  int fds[4];
  size_t nfds = 0;
  size_t rootlen = strlen (root);
  FTSENT *ent;
  while ((ent = fts_read (fts)) != NULL)
    {
      const char *rel = ent->fts_path + rootlen;
      if (*rel == '/')
        rel++;

      if (ent->fts_info != FTS_D
	  && ent->fts_info != FTS_DP
          && ent->fts_info != FTS_F)
        {
          FAIL ("%s: unexpected fts_info %d (fts_errno %d)", ent->fts_path,
                ent->fts_info, ent->fts_errno);
          continue;
        }

      /* The returned entry must be reachable through fts_accpath from the
	 current, possibly virtual, working directory.  */
      int dirfd = (options & FTS_CWDFD) ? fts->fts_cwd_fd : AT_FDCWD;
      struct stat st;
      if (fstatat (dirfd, ent->fts_accpath, &st, AT_SYMLINK_NOFOLLOW) != 0)
        FAIL ("%s: fstatat (%s): %m", ent->fts_path, ent->fts_accpath);
      else
        TEST_VERIFY (st.st_ino == ent->fts_statp->st_ino);

      /* Follow every child of root before fts_read reaches it.  fts_read does
	 not apply the instruction to the first child.  */
      if (ent->fts_level == FTS_ROOTLEVEL && ent->fts_info == FTS_D)
        for (FTSENT *child = fts_children (fts, 0);
	     child != NULL;
             child = child->fts_link)
          TEST_COMPARE (fts_set (fts, child, FTS_FOLLOW), 0);

      /* If fts closed the descriptor of its working directory when it left
	 the followed directory, this open reuses it.  */
      if (ent->fts_level == 1 && ent->fts_info == FTS_DP)
        {
          TEST_VERIFY_EXIT (nfds < array_length (fds));
          fds[nfds++] = xopen ("/dev/null", O_RDONLY, 0);
        }

      if (ent->fts_info != FTS_DP)
        for (size_t i = 0; i < array_length (expected); i++)
          if (strcmp (rel, expected[i]) == 0)
            {
              TEST_VERIFY (!seen[i]);
              seen[i] = true;
            }
    }
  TEST_COMPARE (errno, 0);
  TEST_COMPARE (fts_close (fts), 0);

  for (size_t i = 0; i < array_length (expected); i++)
    if (!seen[i])
      FAIL ("%s/%s was not visited", root, expected[i]);

  /* fts must not close descriptors that it does not own.  */
  for (size_t i = 0; i < nfds; i++)
    if (fcntl (fds[i], F_GETFD) < 0)
      FAIL ("descriptor %d was closed by fts: %m", fds[i]);
    else
      xclose (fds[i]);

  support_descriptors_check (descriptors);
  support_descriptors_free (descriptors);
}

static int
do_test (void)
{
  static const int options[] =
    {
      FTS_PHYSICAL,
      FTS_PHYSICAL | FTS_CWDFD,
      FTS_PHYSICAL | FTS_CWDFD | FTS_DEFER_STAT,
    };

  for (size_t i = 0; i < array_length (options); i++)
    {
      check_walk (options[i], true);
      check_walk (options[i], false);
    }

  return 0;
}

#include <support/test-driver.c>
