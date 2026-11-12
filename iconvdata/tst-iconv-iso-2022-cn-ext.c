/* Verify ISO-2022-CN-EXT does not write out of the bounds.
   Copyright (C) 2024-2025 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <iconv.h>
#include <sys/mman.h>

#include <support/xunistd.h>
#include <support/check.h>
#include <support/next_to_fault.h>
#include <support/support.h>

/* The test sets up a two memory page buffer with the second page marked
   PROT_NONE to trigger a fault if the conversion writes beyond the exact
   expected amount.  Then we carry out various conversions and precisely
   place the start of the output buffer in order to trigger a SIGSEGV if the
   process writes anywhere between 1 and page sized bytes more (only one
   PROT_NONE page is setup as a canary) than expected.  These tests exercise
   all three of the cases in ISO-2022-CN-EXT where the converter must switch
   character sets and may run out of buffer space while doing the
   operation.  */

/* FROM-direction regression tests for bug 34774: a lone SO (0x0e)
   without any announced SO designation.  The error paths must leave
   the input pointer on the offending SO byte, and //IGNORE must skip
   it exactly once.  */

static void
test_lone_so_plain (void)
{
  iconv_t cd = iconv_open ("UTF-8", "ISO-2022-CN-EXT");
  TEST_VERIFY_EXIT (cd != (iconv_t) -1);
  char in[] = { 0x0e };
  char out[8];
  char *ip = in, *op = out;
  size_t il = sizeof in, ol = sizeof out;
  errno = 0;
  TEST_COMPARE (iconv (cd, &ip, &il, &op, &ol), (size_t) -1);
  TEST_COMPARE (errno, EILSEQ);
  TEST_VERIFY (ip == in);
  TEST_COMPARE (il, (size_t) 1);
  TEST_COMPARE (iconv_close (cd), 0);
}

static void
test_so_after_ascii (void)
{
  iconv_t cd = iconv_open ("UTF-8", "ISO-2022-CN-EXT");
  TEST_VERIFY_EXIT (cd != (iconv_t) -1);
  char in[] = { 'A', 0x0e, 'B' };
  char out[8];
  char *ip = in, *op = out;
  size_t il = sizeof in, ol = sizeof out;
  errno = 0;
  TEST_COMPARE (iconv (cd, &ip, &il, &op, &ol), (size_t) -1);
  TEST_COMPARE (errno, EILSEQ);
  TEST_VERIFY (ip == in + 1);
  TEST_COMPARE (il, (size_t) 2);
  TEST_VERIFY (op == out + 1);
  TEST_COMPARE_BLOB (out, op - out, "A", 1);
  TEST_COMPARE (iconv_close (cd), 0);
}

static void
test_lone_so_ignore (void)
{
  iconv_t cd = iconv_open ("UTF-8//IGNORE", "ISO-2022-CN-EXT");
  TEST_VERIFY_EXIT (cd != (iconv_t) -1);
  char in[] = { 0x0e, 'A', 'B' };
  char out[8];
  char *ip = in, *op = out;
  size_t il = sizeof in, ol = sizeof out;
  errno = 0;
  /* With //IGNORE the offending SO must be skipped exactly once and the
     following ASCII must be converted.  Note that glibc still reports
     EILSEQ here: the ignore arm of STANDARD_FROM_LOOP_ERR_HANDLER does
     not reset result, which stays __GCONV_ILLEGAL_INPUT, and the
     iconv() wrapper maps that status to -1 plus EILSEQ
     unconditionally (iconv/iconv.c).  */
  TEST_COMPARE (iconv (cd, &ip, &il, &op, &ol), (size_t) -1);
  TEST_COMPARE (errno, EILSEQ);
  TEST_VERIFY (ip == in + 3);
  TEST_COMPARE (il, (size_t) 0);
  TEST_COMPARE_BLOB (out, op - out, "AB", 2);
  TEST_COMPARE (iconv_close (cd), 0);
}

static void
test_caller_skip_no_underflow (void)
{
  /* A caller that skips the offending byte on EILSEQ and calls
     iconv() again.  With the fix, the first call reports the SO as
     unconsumed input, so the skip drops the SO itself and the rest of
     the input still converts.  */
  iconv_t cd = iconv_open ("UTF-8", "ISO-2022-CN-EXT");
  TEST_VERIFY_EXIT (cd != (iconv_t) -1);
  char in[] = { 0x0e, 'A' };
  char out[8];
  char *ip = in, *op = out;
  size_t il = sizeof in, ol = sizeof out;
  errno = 0;
  TEST_COMPARE (iconv (cd, &ip, &il, &op, &ol), (size_t) -1);
  TEST_COMPARE (errno, EILSEQ);
  /* The SO is the offending byte and must still be unconsumed; do not
     run the caller-side skip unless this precondition holds, or the
     test itself would advance past the end of the input.  */
  TEST_VERIFY_EXIT (ip == in);
  TEST_VERIFY_EXIT (il == 2);
  ++ip;
  --il;
  op = out;
  ol = sizeof out;
  errno = 0;
  TEST_COMPARE (iconv (cd, &ip, &il, &op, &ol), (size_t) 0);
  TEST_COMPARE (il, (size_t) 0);
  TEST_COMPARE ((size_t) (op - out), (size_t) 1);
  TEST_COMPARE_BLOB (out, op - out, "A", 1);
  TEST_COMPARE (iconv_close (cd), 0);
}

static void
test_lone_so_tail_caller_skip (void)
{
  /* A lone SO at the end of the input.  Before the fix the first call
     consumed the SO and left the remaining length at 0, so a caller
     skipping the offending byte decremented that length to
     (size_t) -1.  */
  iconv_t cd = iconv_open ("UTF-8", "ISO-2022-CN-EXT");
  TEST_VERIFY_EXIT (cd != (iconv_t) -1);
  char in[] = { 'A', 0x0e };
  char out[8];
  char *ip = in, *op = out;
  size_t il = sizeof in, ol = sizeof out;
  errno = 0;
  TEST_COMPARE (iconv (cd, &ip, &il, &op, &ol), (size_t) -1);
  TEST_COMPARE (errno, EILSEQ);
  TEST_VERIFY_EXIT (ip == in + 1);
  TEST_VERIFY_EXIT (il == 1);
  ++ip;
  --il;
  op = out;
  ol = sizeof out;
  errno = 0;
  TEST_COMPARE (iconv (cd, &ip, &il, &op, &ol), (size_t) 0);
  TEST_COMPARE (il, (size_t) 0);
  TEST_COMPARE (iconv_close (cd), 0);
}

static void
test_designated_so (void)
{
  /* Positive case: with an announced designation the SO switches to
     the designated set, consumes exactly one byte, and the following
     7-bit GB2312 pair converts.  ESC $ ) A designates GB2312;
     0x52 0x3b is the 7-bit form of the GB2312 code for U+4E00.  */
  iconv_t cd = iconv_open ("UTF-8", "ISO-2022-CN-EXT");
  TEST_VERIFY_EXIT (cd != (iconv_t) -1);
  char in[] = { 0x1b, '$', ')', 'A', 0x0e, 0x52, 0x3b, 0x0f, 'Z' };
  char out[16];
  char *ip = in, *op = out;
  size_t il = sizeof in, ol = sizeof out;
  errno = 0;
  TEST_COMPARE (iconv (cd, &ip, &il, &op, &ol), (size_t) 0);
  TEST_COMPARE (il, (size_t) 0);
  TEST_COMPARE ((size_t) (op - out), (size_t) 4);
  TEST_COMPARE_BLOB (out, 4, "\xe4\xb8\x80Z", 4);
  TEST_COMPARE (iconv_close (cd), 0);
}

static void
test_trailing_so_ignore (void)
{
  /* The shape from bug 34774: a lone SO as the last input byte with
     //IGNORE.  Place the input right before an inaccessible page so
     that a read past the end faults.  */
  struct support_next_to_fault ntf = support_next_to_fault_allocate (2);
  ntf.buffer[0] = 'A';
  ntf.buffer[1] = 0x0e;
  iconv_t cd = iconv_open ("UTF-8//IGNORE", "ISO-2022-CN-EXT");
  TEST_VERIFY_EXIT (cd != (iconv_t) -1);
  char out[8];
  char *ip = ntf.buffer, *op = out;
  size_t il = 2, ol = sizeof out;
  errno = 0;
  TEST_COMPARE (iconv (cd, &ip, &il, &op, &ol), (size_t) -1);
  TEST_COMPARE (errno, EILSEQ);
  TEST_VERIFY (ip == ntf.buffer + 2);
  TEST_COMPARE (il, (size_t) 0);
  TEST_COMPARE_BLOB (out, op - out, "A", 1);
  TEST_COMPARE (iconv_close (cd), 0);
  support_next_to_fault_free (&ntf);
}

static int
do_test (void)
{
  iconv_t cd = iconv_open ("ISO-2022-CN-EXT", "UTF-8");
  TEST_VERIFY_EXIT (cd != (iconv_t) -1);

  char *ntf;
  size_t ntfsize;
  char *outbufbase;
  {
    int pgz = getpagesize ();
    TEST_VERIFY_EXIT (pgz > 0);
    ntfsize = 2 * pgz;

    ntf = xmmap (NULL, ntfsize, PROT_READ | PROT_WRITE, MAP_PRIVATE
		 | MAP_ANONYMOUS, -1);
    xmprotect (ntf + pgz, pgz, PROT_NONE);

    outbufbase = ntf + pgz;
  }

  /* Check if SOdesignation escape sequence does not trigger an OOB write.  */
  {
    char inbuf[] = "\xe4\xba\xa4\xe6\x8d\xa2";

    for (int i = 0; i < 9; i++)
      {
	char *inp = inbuf;
	size_t inleft = sizeof (inbuf) - 1;

	char *outp = outbufbase - i;
	size_t outleft = i;

	TEST_VERIFY_EXIT (iconv (cd, &inp, &inleft, &outp, &outleft)
			  == (size_t) -1);
	TEST_COMPARE (errno, E2BIG);

	TEST_VERIFY_EXIT (iconv (cd, NULL, NULL, NULL, NULL) == 0);
      }
  }

  /* Same as before for SS2designation.  */
  {
    char inbuf[] = "㴽 \xe3\xb4\xbd";

    for (int i = 0; i < 14; i++)
      {
	char *inp = inbuf;
	size_t inleft = sizeof (inbuf) - 1;

	char *outp = outbufbase - i;
	size_t outleft = i;

	TEST_VERIFY_EXIT (iconv (cd, &inp, &inleft, &outp, &outleft)
			  == (size_t) -1);
	TEST_COMPARE (errno, E2BIG);

	TEST_VERIFY_EXIT (iconv (cd, NULL, NULL, NULL, NULL) == 0);
      }
  }

  /* Same as before for SS3designation.  */
  {
    char inbuf[] = "劄 \xe5\x8a\x84";

    for (int i = 0; i < 14; i++)
      {
	char *inp = inbuf;
	size_t inleft = sizeof (inbuf) - 1;

	char *outp = outbufbase - i;
	size_t outleft = i;

	TEST_VERIFY_EXIT (iconv (cd, &inp, &inleft, &outp, &outleft)
			  == (size_t) -1);
	TEST_COMPARE (errno, E2BIG);

	TEST_VERIFY_EXIT (iconv (cd, NULL, NULL, NULL, NULL) == 0);
      }
  }

  TEST_VERIFY_EXIT (iconv_close (cd) != -1);

  xmunmap (ntf, ntfsize);

  test_trailing_so_ignore ();
  test_lone_so_plain ();
  test_so_after_ascii ();
  test_lone_so_ignore ();
  test_caller_skip_no_underflow ();
  test_lone_so_tail_caller_skip ();
  test_designated_so ();

  return 0;
}

#include <support/test-driver.c>
