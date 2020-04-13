/* Check multiple telldir and seekdir.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <support/temp_file.h>
#include <support/support.h>
#include <support/check.h>

/* Some filesystems returns a arbitrary value for d_off direnty entry (ext4
   for instance, where the value is an internal hash key).  The idea of
   create a large number of file is to try trigger a overflow d_off value
   in a entry to check if telldir/seekdir does work corretly in such
   case.  */
static const char *dirname;
static const size_t nfiles = 10240;

static void
do_prepare (int argc, char *argv[])
{
  dirname = support_create_temp_directory ("tst-seekdir2-");

  for (size_t i = 0; i < nfiles; i++)
    {
      int fd = create_temp_file_in_dir ("tempfile.", dirname, NULL);
      TEST_VERIFY_EXIT (fd > 0);
      close (fd);
    }
}
#define PREPARE do_prepare

/* Check for old non Large File Support (LFS).  */
static int
do_test_not_lfs (void)
{
  DIR *dirp;
  struct dirent *dp;
  size_t dirp_count;

  dirp = opendir (dirname);
  TEST_VERIFY_EXIT (dirp != NULL);

  dirp_count = 0;
  for (dp = readdir (dirp);
       dp != NULL;
       dp = readdir (dirp))
    dirp_count++;

  /* The 2 extra files are '.' and '..'.  */
  TEST_COMPARE (dirp_count, nfiles + 2);

  rewinddir (dirp);

  long *tdirp = xmalloc (dirp_count * sizeof (long));
  struct dirent *ddirp = xmalloc (dirp_count * sizeof (struct dirent));

  size_t i = 0;
  do
    {
      tdirp[i] = telldir (dirp);
      dp = readdir (dirp);
      TEST_VERIFY_EXIT (dp != NULL);
      memcpy (&ddirp[i], dp, sizeof (struct dirent));
    } while (++i < dirp_count);

  for (i = 0; i < dirp_count - 1; i++)
    {
      seekdir (dirp, tdirp[i]);
      dp = readdir (dirp);
      TEST_COMPARE (strcmp (dp->d_name, ddirp[i].d_name), 0);
      TEST_COMPARE (dp->d_ino, ddirp[i].d_ino);
      TEST_COMPARE (dp->d_off, ddirp[i].d_off);
    }

  closedir (dirp);

  return 0;
}

/* Same as before but with LFS support.  */
static int
do_test_lfs (void)
{
  DIR *dirp;
  struct dirent64 *dp;
  size_t dirp_count;

  dirp = opendir (dirname);
  TEST_VERIFY_EXIT (dirp != NULL);

  dirp_count = 0;
  for (dp = readdir64 (dirp);
       dp != NULL;
       dp = readdir64 (dirp))
    dirp_count++;

  /* The 2 extra files are '.' and '..'.  */
  TEST_COMPARE (dirp_count, nfiles + 2);

  rewinddir (dirp);

  long *tdirp = xmalloc (dirp_count * sizeof (long));
  struct dirent64 *ddirp = xmalloc (dirp_count * sizeof (struct dirent64));

  size_t i = 0;
  do
    {
      tdirp[i] = telldir (dirp);
      dp = readdir64 (dirp);
      TEST_VERIFY_EXIT (dp != NULL);
      memcpy (&ddirp[i], dp, sizeof (struct dirent64));
    } while (++i < dirp_count);

  for (i = 0; i < dirp_count - 1; i++)
    {
      seekdir (dirp, tdirp[i]);
      dp = readdir64 (dirp);
      TEST_COMPARE (strcmp (dp->d_name, ddirp[i].d_name), 0);
      TEST_COMPARE (dp->d_ino, ddirp[i].d_ino);
      TEST_COMPARE (dp->d_off, ddirp[i].d_off);
    }

  closedir (dirp);

  return 0;
}

static int
do_test (void)
{
  do_test_not_lfs ();
  do_test_lfs ();

  return 0;
}

#include <support/test-driver.c>
