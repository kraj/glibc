/* Test stream positioning in fget*ent (bug 31936).
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
#include <grp.h>
#include <gshadow.h>
#include <mcheck.h>
#include <pwd.h>
#include <shadow.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xstdio.h>

struct input_file
{
  const char *data;
  size_t length;
  off64_t offset;
  bool fail_seek;
};

static ssize_t
read_input (void *closure, char *buffer, size_t length)
{
  struct input_file *file = closure;
  size_t remaining = file->length - file->offset;
  if (length > remaining)
    length = remaining;
  memcpy (buffer, file->data + file->offset, length);
  file->offset += length;
  return length;
}

static int
seek_input (void *closure, off64_t *offset, int whence)
{
  struct input_file *file = closure;
  if (whence == SEEK_CUR)
    {
      /* A zero-offset SEEK_CUR only queries the position and must succeed
         even when actual seeks are set to fail.  */
      TEST_COMPARE (*offset, 0);
      *offset = file->offset;
      return 0;
    }

  TEST_VERIFY_EXIT (whence == SEEK_SET);
  TEST_VERIFY_EXIT (*offset >= 0 && *offset <= (off64_t) file->length);

  /* With fail_seek set, the _r function cannot rewind and fget*ent returns
     NULL with errno set to ESPIPE.  Otherwise, reject a second rewind after
     the _r function has restored the position: the unchanged offset
     identifies the redundant seek that used to lose the reallocated buffer
     in the non-_r wrappers.  */
  if (file->fail_seek || *offset == file->offset)
    {
      /* Inject an I/O error, not a test failure; check_growing checks the
         resulting error or successful read.  */
      errno = EIO;
      return -1;
    }
  file->offset = *offset;
  return 0;
}

static FILE *
open_input (struct input_file *file, bool seekable)
{
  FILE *fp = fopencookie (file, "r", (cookie_io_functions_t)
                         {
                           .read = read_input,
                           .seek = seekable ? seek_input : NULL,
                         });
  TEST_VERIFY_EXIT (fp != NULL);
  /* Make each rewind reach the seek callback without read-ahead.  */
  TEST_COMPARE (setvbuf (fp, NULL, _IONBF, 0), 0);
  return fp;
}

static const char *
read_group (FILE *fp)
{
  struct group *entry = fgetgrent (fp);
  if (entry == NULL)
    return NULL;
  TEST_COMPARE_STRING (entry->gr_name, "entry");
  TEST_COMPARE (entry->gr_gid, 1234);
  TEST_VERIFY (entry->gr_mem[0] == NULL);
  return entry->gr_passwd;
}

static const char *
read_passwd (FILE *fp)
{
  struct passwd *entry = fgetpwent (fp);
  if (entry == NULL)
    return NULL;
  TEST_COMPARE_STRING (entry->pw_name, "entry");
  TEST_COMPARE (entry->pw_uid, 1234);
  TEST_COMPARE (entry->pw_gid, 5678);
  TEST_COMPARE_STRING (entry->pw_dir, "/home/entry");
  TEST_COMPARE_STRING (entry->pw_shell, "/bin/sh");
  return entry->pw_passwd;
}

static const char *
read_shadow (FILE *fp)
{
  struct spwd *entry = fgetspent (fp);
  if (entry == NULL)
    return NULL;
  TEST_COMPARE_STRING (entry->sp_namp, "entry");
  TEST_COMPARE (entry->sp_lstchg, 1);
  TEST_COMPARE (entry->sp_flag, 7);
  return entry->sp_pwdp;
}

static const char *
read_gshadow (FILE *fp)
{
  struct sgrp *entry = fgetsgent (fp);
  if (entry == NULL)
    return NULL;
  TEST_COMPARE_STRING (entry->sg_namp, "entry");
  TEST_VERIFY (entry->sg_adm[0] == NULL);
  TEST_VERIFY (entry->sg_mem[0] == NULL);
  return entry->sg_passwd;
}

static const struct
{
  const char *name;
  const char *(*read) (FILE *);
  const char *suffix;
} tests[] =
  {
    { "fgetgrent", read_group, ":1234:\n" },
    { "fgetpwent", read_passwd, ":1234:5678:Name:/home/entry:/bin/sh\n" },
    { "fgetspent", read_shadow, ":1:2:3:4:5:6:7\n" },
    { "fgetsgent", read_gshadow, "::\n" },
  };

static void
check_growing (size_t test, size_t length, bool fail_seek)
{
  char *password = xmalloc (length + 1);
  memset (password, 'x', length);
  password[length] = '\0';
  char *data = xasprintf ("entry:%s%sentry:x%s", password,
                         tests[test].suffix, tests[test].suffix);
  struct input_file file =
    {
      .data = data,
      .length = strlen (data),
      .fail_seek = fail_seek,
    };
  FILE *fp = open_input (&file, true);

  if (fail_seek)
    {
      /* A required rewind fails here: expect NULL, ESPIPE and a stream
         error, then verify below that reading works after recovery.  */
      errno = 0;
      TEST_VERIFY (tests[test].read (fp) == NULL);
      TEST_COMPARE (errno, ESPIPE);
      TEST_VERIFY (ferror (fp));

      /* Retry after a genuine positioning failure, using the same
         internal buffer.  */
      file.fail_seek = false;
      clearerr (fp);
      TEST_COMPARE (fseeko (fp, 0, SEEK_SET), 0);
    }

  /* Reading must not attempt a redundant rewind, which seek_input
     rejects even after fail_seek has been cleared.  */
  TEST_COMPARE_STRING (tests[test].read (fp), password);
  TEST_COMPARE_STRING (tests[test].read (fp), "x");
  errno = 0;
  TEST_VERIFY (tests[test].read (fp) == NULL);
  TEST_COMPARE (errno, ENOENT);
  TEST_VERIFY (feof (fp));
  TEST_VERIFY (!ferror (fp));

  xfclose (fp);
  free (data);
  free (password);
}

static void
check_nonseekable (size_t test)
{
  char *data = xasprintf ("# Comment\n\nentry:x%s", tests[test].suffix);
  struct input_file file = { .data = data, .length = strlen (data) };
  FILE *fp = open_input (&file, false);

  /* A short entry does not require a rewind.  */
  TEST_COMPARE_STRING (tests[test].read (fp), "x");
  errno = 0;
  TEST_VERIFY (tests[test].read (fp) == NULL);
  TEST_COMPARE (errno, ENOENT);
  TEST_VERIFY (feof (fp));
  TEST_VERIFY (!ferror (fp));

  xfclose (fp);
  free (data);
}

static int
do_test (void)
{
  mtrace ();

  for (size_t i = 0; i < array_length (tests); ++i)
    {
      printf ("info: testing %s\n", tests[i].name);
      /* Exceed the initial 1024-byte buffers to force repeated growth.  */
      check_growing (i, 8192, false);
      /* The static buffers survive the first call; exceed their grown sizes
         so that the seek failure is reached and recovery needs growth.  */
      check_growing (i, 16384, true);
      check_nonseekable (i);
    }
  return 0;
}

#include <support/test-driver.c>
