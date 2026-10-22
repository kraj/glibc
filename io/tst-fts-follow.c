/* Test fts_set with FTS_FOLLOW on a symbolic link to a directory (BZ 34775).
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

     root/aa
     root/link -> target
     root/target/file
     root/target/a/b/c/d/e/file
     root/zz

   The nested directories are deep enough to exhaust the descriptor ring used
   by FTS_CWDFD, so the return from the followed directory can not rely on it.
   The files after "link" in name order check that the walk resumes in the
   right directory.  */
static void
do_prepare (int argc, char **argv)
{
  char *tempdir = support_create_temp_directory ("tst-fts-follow-");
  root = xasprintf ("%s/root", tempdir);
  free (tempdir);
  xmkdir (root, 0700);
  add_temp_file (root);

  static const char *const dirs[] =
    {
      "target",
      "target/a",
      "target/a/b",
      "target/a/b/c",
      "target/a/b/c/d",
      "target/a/b/c/d/e"
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
      "aa",
      "zz",
      "target/file",
      "target/a/b/c/d/e/file"
    };
  for (size_t i = 0; i < array_length (files); i++)
    {
      char *path = xasprintf ("%s/%s", root, files[i]);
      xclose (xopen (path, O_WRONLY | O_CREAT | O_EXCL, 0600));
      add_temp_file (path);
      free (path);
    }

  char *path = xasprintf ("%s/link", root);
  xsymlink ("target", path);
  add_temp_file (path);
  free (path);
}
#define PREPARE do_prepare

static int
compare_name (const FTSENT **a, const FTSENT **b)
{
  return strcmp ((*a)->fts_name, (*b)->fts_name);
}

enum follow_mode
  {
    /* fts_set on the FTS_SL entry returned by fts_read.  */
    follow_returned,
    /* fts_set on the entry from fts_children of root, before fts_read
       reaches it.  */
    follow_children,
  };

static const char *const expected[] =
  {
    "link",
    "link/file",
    "link/a/b/c/d/e/file",
    "target/file",
    "aa",
    "zz"
  };

static void
check_walk (int options, enum follow_mode mode, bool sort)
{
  printf ("info: options=%#x mode=%s sort=%d\n", options,
          mode == follow_returned ? "returned" : "children", sort);

  struct support_descriptors *descriptors = support_descriptors_list ();
  char *cwd = getcwd (NULL, 0);
  TEST_VERIFY_EXIT (cwd != NULL);

  char *paths[] = { root, NULL };
  FTS *fts = fts_open (paths, options, sort ? compare_name : NULL);
  TEST_VERIFY_EXIT (fts != NULL);

  bool seen[array_length (expected)] = { false };
  size_t rootlen = strlen (root);
  bool followed = false;
  FTSENT *ent;
  while ((ent = fts_read (fts)) != NULL)
    {
      const char *rel = ent->fts_path + rootlen;
      if (*rel == '/')
        rel++;

      if (ent->fts_info == FTS_DNR
	  || ent->fts_info == FTS_ERR
          || ent->fts_info == FTS_NS
	  || ent->fts_info == FTS_DC)
        FAIL ("%s: unexpected fts_info %d (fts_errno %d)", ent->fts_path,
              ent->fts_info, ent->fts_errno);

      if (ent->fts_info == FTS_SL)
        {
          /* With follow_children, fts_read still returns the entry once
	     without following it if it is the first child.  */
          TEST_COMPARE_STRING (rel, "link");
          if (mode == follow_returned)
            {
              TEST_VERIFY (!followed);
              TEST_COMPARE (fts_set (fts, ent, FTS_FOLLOW), 0);
              followed = true;
            }
          continue;
        }

      /* The returned entry must be reachable through fts_accpath from the
	 current, possibly virtual, working directory.  */
      int dirfd = (options & FTS_CWDFD) ? fts->fts_cwd_fd : AT_FDCWD;
      {
	struct stat st;
	int flags = strcmp (rel, "link") == 0 ? 0 : AT_SYMLINK_NOFOLLOW;
	if (fstatat (dirfd, ent->fts_accpath, &st, flags) != 0)
	  FAIL ("%s: fstatat (%s): %m", ent->fts_path, ent->fts_accpath);
	else
	  TEST_VERIFY (st.st_ino == ent->fts_statp->st_ino);
      }

      if (mode == follow_children
	  && ent->fts_level == FTS_ROOTLEVEL
          && ent->fts_info == FTS_D)
        {
          FTSENT *child;
          for (child = fts_children (fts, 0); child != NULL;
               child = child->fts_link)
            if (strcmp (child->fts_name, "link") == 0)
              break;
          TEST_VERIFY_EXIT (child != NULL);
          TEST_COMPARE (fts_set (fts, child, FTS_FOLLOW), 0);
          followed = true;
        }

      if (ent->fts_info == FTS_D && strcmp (rel, "link") == 0)
        TEST_VERIFY (followed);

      if (ent->fts_info == FTS_D || ent->fts_info == FTS_F)
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

  char *cwd_after = getcwd (NULL, 0);
  TEST_VERIFY_EXIT (cwd_after != NULL);
  TEST_COMPARE_STRING (cwd, cwd_after);
  free (cwd_after);
  free (cwd);

  support_descriptors_check (descriptors);
  support_descriptors_free (descriptors);
}

static int
do_test (void)
{
  static const int options[] =
    {
      FTS_PHYSICAL,
      FTS_PHYSICAL | FTS_NOCHDIR,
      FTS_PHYSICAL | FTS_COMFOLLOW | FTS_NOCHDIR,
      FTS_PHYSICAL | FTS_CWDFD,
      FTS_PHYSICAL | FTS_CWDFD | FTS_DEFER_STAT,
    };

  for (size_t i = 0; i < array_length (options); i++)
    {
      check_walk (options[i], follow_returned, true);
      check_walk (options[i], follow_returned, false);
      /* With FTS_NOCHDIR, fts_read stats a sibling entry marked with
	 FTS_FOLLOW before it builds the entry's path, so it checks the wrong
	 file.  This is a long-standing issue unrelated to BZ 34775.  */
      if (!(options[i] & FTS_NOCHDIR))
	{
	  check_walk (options[i], follow_children, true);
	  check_walk (options[i], follow_children, false);
	}
    }

  return 0;
}

#include <support/test-driver.c>
