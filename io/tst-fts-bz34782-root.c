/* Check fts_open with root arguments around USHRT_MAX bytes (BZ 34782).
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
#include <fts.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <support/check.h>
#include <support/support.h>

/* FTSENT::fts_namelen and FTSENT::fts_pathlen are unsigned short, so
   fts_open must reject a root argument longer than USHRT_MAX instead of
   truncating its length.  */
static void
check_root (size_t len)
{
  char *root = xmalloc (len + 1);
  memset (root, 'a', len);
  root[len] = '\0';
  char *paths[] = { root, NULL };

  errno = 0;
  FTS *ftsp = fts_open (paths, FTS_PHYSICAL, NULL);
  if (len > USHRT_MAX)
    {
      if (ftsp != NULL)
	{
	  support_record_failure ();
	  printf ("error: fts_open accepted a %zu byte root\n", len);
	  fts_close (ftsp);
	}
      else
	TEST_COMPARE (errno, ENAMETOOLONG);
    }
  else
    {
      TEST_VERIFY_EXIT (ftsp != NULL);
      FTSENT *ent = fts_read (ftsp);
      TEST_VERIFY_EXIT (ent != NULL);
      /* The name does not exist (and is longer than PATH_MAX on Linux),
	 but the entry must still describe it correctly.  */
      TEST_COMPARE (ent->fts_info, FTS_NS);
      TEST_COMPARE (ent->fts_namelen, len);
      TEST_COMPARE (ent->fts_pathlen, len);
      TEST_COMPARE (strnlen (ent->fts_path, len + 1), len);
      TEST_VERIFY (memcmp (ent->fts_path, root, len) == 0);
      TEST_COMPARE (fts_close (ftsp), 0);
    }

  free (root);
}

static int
do_test (void)
{
  check_root (USHRT_MAX);
  check_root (USHRT_MAX + 1);
  check_root (USHRT_MAX + 2);
  check_root (2 * (USHRT_MAX + 1) + 10);
  return 0;
}

#include <support/test-driver.c>
