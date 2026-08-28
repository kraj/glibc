/* Test JISX0213 combining character conversion progress (bug 34556, bug 34568).
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

/* Certain JISX0213 byte sequences map to a combining sequence, for
   example U+304B (HIRAGANA LETTER KA) followed by U+309A (COMBINING
   SEMI-VOICED SOUND MARK).  When converting to internal encoding
   (actually UTF-32) with a small output buffer, the first code point
   is emitted and the second is queued in the converter state.  This
   test verifies that the queued code point is consumed exactly once
   on retry, so that the conversion makes progress and terminates.  */

#include <errno.h>
#include <iconv.h>
#include <stdio.h>
#include <string.h>

#include <support/check.h>
#include <support/support.h>

static void
test_one (const char *charset, const char *input, size_t outbufsize)
{
  printf ("info: %s: testing output buffer size %zu\n", charset, outbufsize);

  /* Expected UTF-32 output.  */
  static const wchar_t expected[] = { 0x304b, 0x309a, 'A' };

  /* Use WCHAR_T encoding to avoid the BOM.  */
  iconv_t cd = iconv_open ("WCHAR_T", charset);
  TEST_VERIFY_EXIT (cd != (iconv_t) -1);

  char result[64];
  size_t result_len = 0;

  char *inptr = (char *) input;
  size_t inleft = strlen (input);

  char outbuf[64];

  int iterations = 0;
  while (inleft > 0)
    {
      char *outptr = outbuf;
      size_t outleft = outbufsize;
      size_t inleft_before = inleft;

      size_t ret = iconv (cd, &inptr, &inleft, &outptr, &outleft);
      size_t produced = outptr - outbuf;

      TEST_VERIFY_EXIT (result_len + produced <= sizeof (result));
      memcpy (result + result_len, outbuf, produced);
      result_len += produced;

      if (ret == (size_t) -1 && errno == E2BIG)
	{
	  if (produced == 0 && inleft == inleft_before)
	    {
	      /* Output buffer too small for a single code point.  */
	      TEST_VERIFY_EXIT (outbufsize < 4);
	      break;
	    }
	  /* Bound iterations to detect non-progress bugs.  */
	  if (++iterations < 10)
	    continue;
	  else
	    {
	      FAIL ("%s: no progress", charset);
	      goto out;
	    }
	}
      if (ret == (size_t) -1)
	FAIL_EXIT1 ("outbufsize %zu: iconv: %m", outbufsize);
      break;
    }

  /* Flush pending converter state.  */
  {
    char *outptr = outbuf;
    size_t outleft = outbufsize;
    size_t ret = iconv (cd, NULL, NULL, &outptr, &outleft);
    TEST_VERIFY (ret == 0);
    size_t produced = outptr - outbuf;
    memcpy (result + result_len, outbuf, produced);
    result_len += produced;
  }

  if (outbufsize >= 4)
    {
      TEST_COMPARE (inleft, 0);
      TEST_COMPARE_BLOB (result, result_len,
			 expected, sizeof (expected));
    }

 out:
  TEST_VERIFY_EXIT (iconv_close (cd) == 0);
}

static int
do_test (void)
{
  for (size_t outbufsize = 1; outbufsize <= 16; outbufsize++)
    {
      test_one ("EUC-JISX0213", "\244\367A", outbufsize);
      test_one ("SHIFT_JISX0213", "\202\365A", outbufsize);
    }
  return 0;
}

#include <support/test-driver.c>
