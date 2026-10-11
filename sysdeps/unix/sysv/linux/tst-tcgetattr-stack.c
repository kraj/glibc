/* Check that tcgetattr does not return uninitialized stack contents.
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

#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <support/check.h>
#include <support/tty.h>

/* Fill the stack below the caller with PATTERN, so that whatever
   tcgetattr leaves uninitialized in its own frame differs between
   two calls.  */
static void __attribute__ ((noinline, noclone))
dirty_stack (int pattern)
{
  char buf[4096];
  memset (buf, pattern, sizeof buf);
  __asm__ volatile ("" : : "r" (buf) : "memory");
}

static void
get (int fd, int pattern, struct termios *tio)
{
  memset (tio, pattern, sizeof *tio);
  dirty_stack (pattern);
  TEST_COMPARE (tcgetattr (fd, tio), 0);
}

static int
do_test (void)
{
  struct termios a, b;
  int outer, inner;

  support_openpty (&outer, &inner, NULL, NULL, NULL);

  get (inner, 0x5a, &a);
  get (inner, 0xa5, &b);
  TEST_COMPARE_BLOB (&a, sizeof a, &b, sizeof b);

  cfmakeraw (&a);
  TEST_COMPARE (tcsetattr (inner, TCSANOW, &a), 0);
  get (inner, 0x5a, &a);
  get (inner, 0xa5, &b);
  TEST_COMPARE_BLOB (&a, sizeof a, &b, sizeof b);

  close (inner);
  close (outer);
  return 0;
}

#include <support/test-driver.c>
