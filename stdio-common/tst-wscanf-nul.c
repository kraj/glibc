/* Test that wide scanf %s and %[ store a single null terminator.
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
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <support/check.h>
#include <support/next_to_fault.h>
#include <support/support.h>
#include <support/test-driver.h>
#include <support/xstdio.h>

struct test
{
  const wchar_t *input;
  const wchar_t *format;
  /* The same conversion with the 'm' modifier.  */
  const wchar_t *mformat;
  /* Multibyte result, without the terminating null byte.  */
  const char *expected;
};

static const struct test tests_c[] =
  {
    { L"abcd", L"%4s", L"%4ms", "abcd" },
    { L"abcdef", L"%4s", L"%4ms", "abcd" },
    { L"ab", L"%4s", L"%4ms", "ab" },
    { L"abcd", L"%s", L"%ms", "abcd" },
    { L"a", L"%1s", L"%1ms", "a" },
    { L"abcd", L"%4[a-z]", L"%4m[a-z]", "abcd" },
    { L"abcdef", L"%4[a-z]", L"%4m[a-z]", "abcd" },
    { L"abcd", L"%[^x]", L"%m[^x]", "abcd" },
  };

static const struct test tests_utf8[] =
  {
    { L"\u00e4\u00f6\u00fc", L"%3s", L"%3ms", "\xc3\xa4\xc3\xb6\xc3\xbc" },
    { L"\u00e4\u00f6\u00fcx", L"%3s", L"%3ms", "\xc3\xa4\xc3\xb6\xc3\xbc" },
    { L"\u00e4\u00f6\u00fc", L"%s", L"%ms", "\xc3\xa4\xc3\xb6\xc3\xbc" },
    { L"\u00e4\u00f6\u00fc", L"%3[^x]", L"%3m[^x]",
      "\xc3\xa4\xc3\xb6\xc3\xbc" },
    { L"\u00e4\u00f6\u00fcx", L"%[^x]", L"%m[^x]",
      "\xc3\xa4\xc3\xb6\xc3\xbc" },
  };

/* Store the conversion into a buffer of exactly the size required for
   the expected result and its terminator, followed by an inaccessible
   page, so a write past the terminator faults.  */
static void
check_buffer (const struct test *t, FILE *fp)
{
  size_t size = strlen (t->expected) + 1;
  struct support_next_to_fault ntf = support_next_to_fault_allocate (size);
  memset (ntf.buffer, 0xff, size);

  int r = fp == NULL
	  ? swscanf (t->input, t->format, ntf.buffer)
	  : fwscanf (fp, t->format, ntf.buffer);
  TEST_COMPARE (r, 1);
  TEST_COMPARE_BLOB (ntf.buffer, size, t->expected, size);

  support_next_to_fault_free (&ntf);
}

/* Same as check_buffer, but with the 'm' modifier allocating the
   result.  */
static void
check_malloc (const struct test *t, FILE *fp)
{
  char *p = NULL;
  int r = fp == NULL
	  ? swscanf (t->input, t->mformat, &p)
	  : fwscanf (fp, t->mformat, &p);
  TEST_COMPARE (r, 1);
  TEST_COMPARE_STRING (p, t->expected);
  free (p);
}

static void
check_test (const struct test *t)
{
  if (test_verbose > 0)
    printf ("info: input \"%ls\" format \"%ls\"\n", t->input, t->format);

  check_buffer (t, NULL);
  check_malloc (t, NULL);

  /* fmemopen streams can not be wide oriented, so use a temporary
     file instead.  */
  FILE *fp = tmpfile ();
  TEST_VERIFY_EXIT (fp != NULL);
  TEST_VERIFY_EXIT (fputws (t->input, fp) >= 0);
  rewind (fp);
  check_buffer (t, fp);
  rewind (fp);
  check_malloc (t, fp);
  xfclose (fp);
}

/* Exercise the 'm' modifier buffer growth for a range of input
   lengths, where each input character converts to CHARLEN bytes.  */
static void
check_alloc (wchar_t wc, const char *mb, size_t charlen)
{
  enum { maxlen = 300 };
  wchar_t input[maxlen + 1];
  char expected[maxlen * MB_LEN_MAX + 1];

  for (size_t len = 1; len <= maxlen; len++)
    {
      for (size_t i = 0; i < len; i++)
	{
	  input[i] = wc;
	  memcpy (expected + i * charlen, mb, charlen);
	}
      input[len] = L'\0';
      expected[len * charlen] = '\0';

      char *p = NULL;
      TEST_COMPARE (swscanf (input, L"%ms", &p), 1);
      TEST_COMPARE_STRING (p, expected);
      free (p);

      p = NULL;
      TEST_COMPARE (swscanf (input, L"%m[^x]", &p), 1);
      TEST_COMPARE_STRING (p, expected);
      free (p);
    }
}

static int
do_test (void)
{
  for (size_t i = 0; i < array_length (tests_c); i++)
    check_test (&tests_c[i]);
  check_alloc (L'a', "a", 1);

  xsetlocale (LC_ALL, "de_DE.UTF-8");
  for (size_t i = 0; i < array_length (tests_c); i++)
    check_test (&tests_c[i]);
  for (size_t i = 0; i < array_length (tests_utf8); i++)
    check_test (&tests_utf8[i]);
  check_alloc (L'a', "a", 1);
  check_alloc (L'\u00e4', "\xc3\xa4", 2);
  check_alloc (L'\u20ac', "\xe2\x82\xac", 3);

  return 0;
}

#include <support/test-driver.c>
