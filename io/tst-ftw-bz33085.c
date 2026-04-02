/* Test if nftw correctly handles readdir errors.
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

#include <ftw.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>

#include <support/check.h>
#include <support/fuse.h>
#include <support/support.h>

/* The test stresses the readdir calls from nftw, where failures should not
   handled as end of stream.  The first is at 'ftw_dir'
   (FTW_STATE_STREAM_LOOP) for the default entries read.  The another one is
   at open_dir_stream where it is triggered when there is a file description
   exaustion and the code must close an existing stream to make room for the
   new subdirectory stream.  */

static _Atomic bool readdir_triggered = false;

static void
fuse_thread (struct support_fuse *f, void *closure)
{
  struct fuse_in_header *inh;

  while ((inh = support_fuse_next (f)) != NULL)
    {
      switch (inh->opcode)
        {
        case FUSE_GETATTR:
          {
            /* We need to respond for both the root (1) and dir1 (2) */
            if (inh->nodeid == 1 || inh->nodeid == 2)
              {
                struct fuse_attr_out out = { 0 };
                out.attr_valid = 60;
                out.attr.ino = inh->nodeid;
                out.attr.mode = S_IFDIR | 0755;
                out.attr.nlink = 3; /* Force nftw to look for subdirs */
                support_fuse_reply (f, &out, sizeof (out));
              }
            else
              support_fuse_reply_error (f, ENOENT);
            break;
          }

        case FUSE_LOOKUP:
          {
            const char *name = (const char *) (inh + 1);
            if (strcmp (name, "dir1") == 0)
              {
                struct fuse_entry_out out = { 0 };
                out.nodeid = 2;
                out.attr_valid = 60;
                out.entry_valid = 60;
                out.attr.ino = 2;
                out.attr.mode = S_IFDIR | 0755;
                out.attr.nlink = 2;
                support_fuse_reply (f, &out, sizeof (out));
              }
            else
              support_fuse_reply_error (f, ENOENT);
            break;
          }

        case FUSE_OPENDIR:
          {
            struct fuse_open_out out = { 0 };
            out.fh = inh->nodeid;
            support_fuse_reply (f, &out, sizeof (out));
            break;
          }

        case FUSE_READDIR:
          {
            const struct fuse_read_in *rin = support_fuse_cast (READ, inh);

            if (inh->nodeid == 1) /* Reading the Root directory */
              {
                if (rin->offset == 0)
                  {
                    /* First readdir, this happens in FTW_STATE_STREAM_LOOP.
                       We yield "dir1", prompting nftw to descend.  */
                    char buf[256] = { 0 };
                    struct fuse_dirent *d = (struct fuse_dirent *) buf;

                    d->ino = 2;
                    d->off = 1;
                    d->type = DT_DIR;
                    d->namelen = 4;
                    strcpy (d->name, "dir1");

                    size_t d_size =
		      FUSE_DIRENT_ALIGN (sizeof (struct fuse_dirent)
					 + d->namelen);
                    support_fuse_reply (f, buf, d_size);
                  }
                else
                  {
		    /* Second readdir, this ONLY happens inside
		       open_dir_stream() when nftw tries to cache the
		       remaining entries before closing the stream to descend
		       into "dir1".  */
                    readdir_triggered = true;
                    support_fuse_reply_error (f, EIO);
                  }
              }
            else
              /* Subdirectory logic (shouldn't be reached in this test) */
              support_fuse_reply_empty (f);
            break;
          }

        case FUSE_READDIRPLUS:
          support_fuse_reply_error (f, EIO);
          break;

        case FUSE_ACCESS:
        case FUSE_RELEASEDIR:
          support_fuse_reply_empty (f);
          break;

        default:
          support_fuse_reply_error (f, EIO);
        }
    }
}

static int
nftw_cb (const char *fpath, const struct stat *sb, int typeflag,
	 struct FTW *ftwbuf)
{
  return 0;
}

static int
do_test (void)
{
  support_fuse_init ();
  struct support_fuse *f = support_fuse_mount (fuse_thread, NULL);

  {
    /* This forces nftw to immediately exhaust its FD limit when it tries
       to descend into 'dir1', forcing it into the open_dir_stream fallback
       loop. */
    errno = 0;
    readdir_triggered = false;
    int ret = nftw (support_fuse_mountpoint (f), nftw_cb, 1, FTW_PHYS);
    /* Assert that we successfully hit the caching __readdir64 block */
    TEST_VERIFY (readdir_triggered);

    /* Assert that nftw correctly aborted and propagated the EIO */
    TEST_COMPARE (ret, -1);
    TEST_COMPARE (errno, EIO);
  }

  {
    /* Use a high descriptor count (10) so nftw doesn't fall back to
       caching */
    errno = 0;
    readdir_triggered = false;
    int ret = nftw (support_fuse_mountpoint (f), nftw_cb, 10, FTW_PHYS);
    /* Assert that the second readdir in the main loop was actually hit */
    TEST_VERIFY (readdir_triggered);

    /* Assert that nftw correctly aborts and propagates the EIO
       This will fail until the `#if 0` block in ftw.c is patched) */
    TEST_COMPARE (ret, -1);
    TEST_COMPARE (errno, EIO);
  }

  support_fuse_unmount (f);
  return 0;
}

#include <support/test-driver.c>
