/* Tests for the procutils line reader.
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
#include <fcntl.h>
#include <procutils.h>
#include <string.h>
#include <support/check.h>
#include <support/temp_file.h>
#include <support/xunistd.h>

enum
{
  max_lines = 8,
  max_line_len = 63,
};

struct capture
{
  int n;
  /* Make the closure stop the read after this number of lines, or 0 to
     always read the whole file.  */
  int stop_at;
  char lines[max_lines][max_line_len + 1];
};

static int
capture_line (const char *line, void *arg)
{
  struct capture *c = arg;
  TEST_VERIFY_EXIT (c->n < max_lines);
  TEST_VERIFY_EXIT (strlen (line) <= max_line_len);
  strcpy (c->lines[c->n++], line);
  return c->n == c->stop_at ? 1 : 0;
}

static char *tempfile;

static void
run_test (const char *content, char *buffer, size_t buffer_size, int stop_at,
	  enum procutils_read_result_t expected_ret,
	  const char *const expected_lines[],
	  int expected_n)
{
  int fd = xopen (tempfile, O_WRONLY | O_TRUNC, 0600);
  xwrite (fd, content, strlen (content));
  xclose (fd);

  struct capture c = { .stop_at = stop_at };
  TEST_COMPARE (__libc_procutils_read_file (tempfile, buffer, buffer_size,
					    capture_line, &c),
		expected_ret);
  TEST_COMPARE (c.n, expected_n);
  for (int i = 0; i < expected_n && i < c.n; i++)
    TEST_COMPARE_STRING (c.lines[i], expected_lines[i]);
}

static int
do_test (void)
{
  xclose (create_temp_file ("tst-procutils", &tempfile));

  char buf64[64];
  /* A small buffer, so lines longer than 7 characters are truncated.  */
  char buf8[8];

  /* Regular lines with a trailing newline, including an empty line.  */
  run_test ("line1\n\nline3\n", buf64, sizeof buf64, 0, procutils_read_eof,
	    (const char *const []) { "line1", "", "line3" }, 3);

  /* Last line without a trailing newline.  */
  run_test ("foo\nbar", buf64, sizeof buf64, 0, procutils_read_eof,
	    (const char *const []) { "foo", "bar" }, 2);

  /* Empty file.  */
  run_test ("", buf64, sizeof buf64, 0, procutils_read_eof, NULL, 0);

  /* A line longer than the buffer is passed truncated to
     buffer_size - 1 characters and the remainder is discarded.  */
  run_test ("abcdefghijklm\nxy\n", buf8, sizeof buf8, 0, procutils_read_eof,
	    (const char *const []) { "abcdefg", "xy" }, 2);

  /* A long line spanning multiple buffer refills.  */
  run_test ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\nend\n", buf8, sizeof buf8, 0,
	    procutils_read_eof,
	    (const char *const []) { "aaaaaaa", "end" }, 2);

  /* A line of exactly buffer_size - 1 characters is not truncated.  */
  run_test ("1234567\n89\n", buf8, sizeof buf8, 0, procutils_read_eof,
	    (const char *const []) { "1234567", "89" }, 2);

  /* A truncated line without a trailing newline at the end of file.  */
  run_test ("abcdefghijklm", buf8, sizeof buf8, 0, procutils_read_eof,
	    (const char *const []) { "abcdefg" }, 1);

  /* The closure stopping the read early.  */
  run_test ("one\ntwo\nthree\n", buf64, sizeof buf64, 2, procutils_read_stop,
	    (const char *const []) { "one", "two" }, 2);

  /* Nonexistent file.  */
  {
    struct capture c = { 0 };
    errno = 0;
    TEST_COMPARE (__libc_procutils_read_file ("/tst-procutils-nonexistent",
					      buf64, sizeof buf64,
					      capture_line, &c),
		  procutils_read_error);
    TEST_COMPARE (errno, ENOENT);
    TEST_COMPARE (c.n, 0);
  }

  return 0;
}

#include <support/test-driver.c>
