/* Test __nss_readline leading whitespace stripping (bug 34765).
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
#include <nss_files.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <support/check.h>

struct test_case
{
  const char *input;
  const char *expected;
  const char *expected_shell;
};

static const struct test_case tests[] =
  {
   { "root:x:0:0:root:/root:/bin/sh\n",
     "root:x:0:0:root:/root:/bin/sh\n", "/bin/sh" },
   { " daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin\n",
     "daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin\n", "/usr/sbin/nologin" },
   { "\tbin:x:2:2:bin:/bin:/usr/sbin/nologin\n",
     "bin:x:2:2:bin:/bin:/usr/sbin/nologin\n", "/usr/sbin/nologin" },
   { "   nobody:x:65534:65534:nobody:/nonexistent:/usr/sbin/nologin\n",
     "nobody:x:65534:65534:nobody:/nonexistent:/usr/sbin/nologin\n", "/usr/sbin/nologin" },
   { "\t  sync:x:4:65534:sync:/bin:/bin/sync\n",
     "sync:x:4:65534:sync:/bin:/bin/sync\n", "/bin/sync" },
   { "  root:x:0:0:root:/root:/bin/sh",
     "root:x:0:0:root:/root:/bin/sh", "/bin/sh" },
  };

static int
do_test (void)
{
  for (size_t i = 0; i < array_length (tests); ++i)
    {
      FILE *fp = fmemopen ((void *) tests[i].input,
			   strlen (tests[i].input), "r");
      TEST_VERIFY_EXIT (fp != NULL);

      char buf[256];
      off64_t offset;
      int ret = __nss_readline (fp, buf, sizeof (buf), &offset);
      TEST_COMPARE (ret, 0);
      TEST_COMPARE_STRING (buf, tests[i].expected);

      /* Verify the same line through a public interface.  */
      rewind (fp);
      struct passwd result;
      struct passwd *rp;
      char buffer[1024];
      ret = fgetpwent_r (fp, &result, buffer, sizeof (buffer), &rp);
      TEST_COMPARE (ret, 0);
      TEST_VERIFY (rp != NULL);
      TEST_COMPARE_STRING (result.pw_shell, tests[i].expected_shell);

      fclose (fp);
    }

  return 0;
}

#include <support/test-driver.c>
