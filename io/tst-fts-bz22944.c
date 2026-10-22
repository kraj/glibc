/* Check that fts does not fail with very long paths.
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

#include <fts.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <support/check.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/xunistd.h>

#ifndef BASENAME
# define BASENAME "tst-fts-bz22944."
# define NESTED_DEPTH 150
#endif

enum { nested_depth = NESTED_DEPTH };
static const char dir_name[] = { [0 ... 254] = 'A', '\0' };

static void
do_cleanup (void)
{
  xchdir ("..");
  for (int i = 0; i < nested_depth; i++)
    {
      remove (dir_name);
      xchdir ("..");
    }
  remove (dir_name);
}
#define CLEANUP_HANDLER do_cleanup

static void
check_mkdir (const char *path)
{
  int r = mkdir (path, 0700);
  /* Some filesystem such as overlayfs does not support larger path required
     to trigger the internal buffer reallocation.  */
  if (r != 0)
    {
      if (errno == ENAMETOOLONG)
	FAIL_UNSUPPORTED ("the filesystem does not support the required"
			  "large path");
      else
	FAIL_EXIT1 ("mkdir (\"%s\", 0%o): %m", path, 0700);
    }
}

int
do_test (void)
{
  char *tempdir = support_create_temp_directory (BASENAME);
  size_t tempdir_len = strlen (tempdir);
  size_t level_len = 1 + strlen (dir_name);

  /* The fts_path of the deepest directory.  The fts_path of a directory at
     level N is its first TEMPDIR_LEN + N * LEVEL_LEN bytes.  */
  size_t full_path_len = tempdir_len + level_len * nested_depth;
  char *full_path = xmalloc (full_path_len + 1);
  char *p = stpcpy (full_path, tempdir);

  xchdir (tempdir);
  for (int i = 0; i < nested_depth; i++)
    {
      check_mkdir (dir_name);
      xchdir (dir_name);
      *p++ = '/';
      p = stpcpy (p, dir_name);
    }

  /* FTSENT::fts_pathlen is an unsigned short, so fts must stop with
     ENAMETOOLONG instead of returning a path longer than USHRT_MAX.  */
  int expected_dirs = nested_depth + 1;
  int expected_errno = 0;
  if (full_path_len > USHRT_MAX)
    {
      expected_dirs = 1 + (USHRT_MAX - tempdir_len) / level_len;
      expected_errno = ENAMETOOLONG;
    }

  char *paths[] = { tempdir , 0 };
  FTS *ftsp = fts_open (paths, FTS_XDEV | FTS_COMFOLLOW | FTS_PHYSICAL, 0);
  TEST_VERIFY_EXIT (ftsp != NULL);

  int num_dirs = 0;
  bool path_mismatch = false;

  FTSENT *ent;
  while ((ent = fts_read(ftsp)) != 0)
    {
      if (ent->fts_info != FTS_D && ent->fts_info != FTS_DP)
	continue;
      if (ent->fts_info == FTS_D)
	num_dirs++;

      /* Stop at the first wrong path, since the paths are large.  */
      size_t len = tempdir_len + level_len * ent->fts_level;
      if (ent->fts_pathlen != len
	  || memcmp (ent->fts_path, full_path, len) != 0
	  || ent->fts_path[len] != '\0')
	{
	  support_record_failure ();
	  printf ("error: wrong fts_path for level %d (fts_pathlen %u, "
		  "expected %zu)\n", ent->fts_level, ent->fts_pathlen, len);
	  path_mismatch = true;
	  break;
	}
    }
  int err = errno;

  TEST_COMPARE (fts_close (ftsp), 0);

  do_cleanup ();

  if (!path_mismatch)
    {
      TEST_COMPARE (num_dirs, expected_dirs);
      TEST_COMPARE (err, expected_errno);
    }

  free (full_path);
  return 0;
}

#include <support/test-driver.c>
