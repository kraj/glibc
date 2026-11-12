/* Verify IBM1390/IBM1399 do not read past the input bounds.
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
#include <errno.h>
#include <iconv.h>

#include <support/check.h>
#include <support/next_to_fault.h>
#include <support/support.h>

static void
test_ibm139x (const char *coding)
{
  /* Use WCHAR_T as input encoding so that the encoder reads directly from
     the input buffer.  */
  iconv_t cd = iconv_open (coding, "WCHAR_T");
  TEST_VERIFY_EXIT (cd != (iconv_t) -1);

  /* Place a combining base one byte before the end of the buffer, to
     check that the encoder does not read past the end of it.  */
  struct support_next_to_fault ntf = support_next_to_fault_allocate (9);
  memcpy (ntf.buffer, L"a\x00E6", 8);
  ntf.buffer[8] = 0;

  char *inp = ntf.buffer;
  size_t inleft = 9;

  char outbuf[32];
  char *outp = outbuf;
  size_t outleft = sizeof (outbuf);

  TEST_COMPARE (iconv (cd, &inp, &inleft, &outp, &outleft), (size_t) -1);
  TEST_COMPARE (errno, EINVAL);
  TEST_COMPARE (inleft, 1);
  TEST_VERIFY (inp == ntf.buffer + 8);
  TEST_COMPARE (iconv_close (cd), 0);
  support_next_to_fault_free (&ntf);
}

static int
do_test (void)
{
  test_ibm139x ("IBM1390");
  test_ibm139x ("IBM1399");

  return 0;
}

#include <support/test-driver.c>
