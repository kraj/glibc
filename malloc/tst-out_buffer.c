/* Tests for struct out_buffer.
   Copyright (C) 2017 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <arpa/inet.h>
#include <out_buffer.h>
#include <stdlib.h>
#include <string.h>
#include <support/check.h>
#include <support/support.h>
#include <stdio.h>

/* Return the start pointer of the buffer.  */
#define START(buf) ((void *) (buf)->__out_buffer_current)

/* Return true if PTR is sufficiently aligned for TYPE.  */
#define IS_ALIGNED(ptr, type) \
  ((((uintptr_t) ptr) & (__out_buffer_assert_align (__alignof (type)) -1)) \
   == 0)

/* Structure with non-power-of-two size.  */
struct twelve
{
  uint32_t buffer[3];
};
_Static_assert (sizeof (struct twelve) == 12, "struct twelve");

/* Check for success obtaining empty arrays.  Does not assume the
   buffer is empty.  */
static void
test_empty_array (struct out_buffer refbuf)
{
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get_bytes (&buf, 0) != NULL);
    TEST_VERIFY (!out_buffer_has_failed (&buf));
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get_array (&buf, char, 0) != NULL);
    TEST_VERIFY (!out_buffer_has_failed (&buf));
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get_array (&buf, double, 0) != NULL);
    TEST_VERIFY (!out_buffer_has_failed (&buf));
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get_array (&buf, struct twelve, 0) != NULL);
    TEST_VERIFY (!out_buffer_has_failed (&buf));
  }
}

/* Test allocation of impossibly large arrays.  */
static void
test_impossible_array (struct out_buffer refbuf)
{
  printf ("info: %s: current=0x%llx end=0x%llx\n",
          __func__, (unsigned long long) refbuf.__out_buffer_current,
          (unsigned long long) refbuf.__out_buffer_end);
  static const size_t counts[] =
    { SIZE_MAX, SIZE_MAX - 1, SIZE_MAX - 2, SIZE_MAX - 3, SIZE_MAX - 4,
      SIZE_MAX / 2, SIZE_MAX / 2 + 1, SIZE_MAX / 2 - 1, 0};

  for (int i = 0; counts[i] != 0; ++i)
    {
      size_t count = counts[i];
      printf ("info: %s: count=%zu\n", __func__, count);
      {
        struct out_buffer buf = refbuf;
        TEST_VERIFY (out_buffer_get_bytes (&buf, count) == NULL);
        TEST_VERIFY (out_buffer_has_failed (&buf));
      }
      {
        struct out_buffer buf = refbuf;
        TEST_VERIFY (out_buffer_get_array (&buf, char, count) == NULL);
        TEST_VERIFY (out_buffer_has_failed (&buf));
      }
      {
        struct out_buffer buf = refbuf;
        TEST_VERIFY (out_buffer_get_array (&buf, short, count) == NULL);
        TEST_VERIFY (out_buffer_has_failed (&buf));
      }
      {
        struct out_buffer buf = refbuf;
        TEST_VERIFY (out_buffer_get_array (&buf, double, count) == NULL);
        TEST_VERIFY (out_buffer_has_failed (&buf));
      }
      {
        struct out_buffer buf = refbuf;
        TEST_VERIFY (out_buffer_get_array (&buf, struct twelve, count)
                     == NULL);
        TEST_VERIFY (out_buffer_has_failed (&buf));
      }
    }
}

/* Check for failure to obtain anything from a failed buffer.  */
static void
test_after_failure (struct out_buffer refbuf)
{
  TEST_VERIFY (out_buffer_has_failed (&refbuf));
  {
    struct out_buffer buf = refbuf;
    out_buffer_add_byte (&buf, 17);
    TEST_VERIFY (out_buffer_has_failed (&buf));
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get (&buf, char) == NULL);
    TEST_VERIFY (out_buffer_has_failed (&buf));
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get (&buf, double) == NULL);
    TEST_VERIFY (out_buffer_has_failed (&buf));
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get (&buf, struct twelve) == NULL);
    TEST_VERIFY (out_buffer_has_failed (&buf));
  }

  test_impossible_array (refbuf);
  for (int count = 0; count <= 4; ++count)
    {
      {
        struct out_buffer buf = refbuf;
        TEST_VERIFY (out_buffer_get_bytes (&buf, count) == NULL);
        TEST_VERIFY (out_buffer_has_failed (&buf));
      }
      {
        struct out_buffer buf = refbuf;
        TEST_VERIFY (out_buffer_get_array (&buf, char, count) == NULL);
        TEST_VERIFY (out_buffer_has_failed (&buf));
      }
      {
        struct out_buffer buf = refbuf;
        TEST_VERIFY (out_buffer_get_array (&buf, double, count) == NULL);
        TEST_VERIFY (out_buffer_has_failed (&buf));
      }
      {
        struct out_buffer buf = refbuf;
        TEST_VERIFY (out_buffer_get_array (&buf, struct twelve, count)
                     == NULL);
        TEST_VERIFY (out_buffer_has_failed (&buf));
      }
    }
}

static void
test_empty (struct out_buffer refbuf)
{
  TEST_VERIFY (!out_buffer_has_failed (&refbuf));
  test_empty_array (refbuf);
  test_impossible_array (refbuf);

  /* Failure to obtain non-empty objects.  */
  {
    struct out_buffer buf = refbuf;
    out_buffer_add_byte (&buf, 17);
    test_after_failure (buf);
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get (&buf, char) == NULL);
    test_after_failure (buf);
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get (&buf, double) == NULL);
    test_after_failure (buf);
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get (&buf, struct twelve) == NULL);
    test_after_failure (buf);
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get_array (&buf, char, 1) == NULL);
    test_after_failure (buf);
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get_array (&buf, double, 1) == NULL);
    test_after_failure (buf);
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get_array (&buf, struct twelve, 1) == NULL);
    test_after_failure (buf);
  }
}

static void
test_size_1 (struct out_buffer refbuf)
{
  TEST_VERIFY (!out_buffer_has_failed (&refbuf));
  test_empty_array (refbuf);
  test_impossible_array (refbuf);

  /* Success adding a single byte.  */
  {
    struct out_buffer buf = refbuf;
    out_buffer_add_byte (&buf, 17);
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    test_empty (buf);
  }
  TEST_VERIFY (memcmp (START (&refbuf), "\x11", 1) == 0);
  {
    struct out_buffer buf = refbuf;
    signed char *ptr = out_buffer_get (&buf, signed char);
    TEST_VERIFY_EXIT (ptr != NULL);
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    *ptr = 126;
    test_empty (buf);
  }
  TEST_VERIFY (memcmp (START (&refbuf), "\176", 1) == 0);
  {
    struct out_buffer buf = refbuf;
    char *ptr = out_buffer_get_array (&buf, char, 1);
    TEST_VERIFY_EXIT (ptr != NULL);
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    *ptr = (char) 253;
    test_empty (buf);
  }
  TEST_VERIFY (memcmp (START (&refbuf), "\xfd", 1) == 0);

  /* Failure with larger objects.  */
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get (&buf, short) == NULL);
    test_after_failure (buf);
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get (&buf, double) == NULL);
    test_after_failure (buf);
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get (&buf, struct twelve) == NULL);
    test_after_failure (buf);
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get_array (&buf, short, 1) == NULL);
    test_after_failure (buf);
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get_array (&buf, double, 1) == NULL);
    test_after_failure (buf);
  }
  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get_array (&buf, struct twelve, 1) == NULL);
    test_after_failure (buf);
  }
}

static void
test_size_2 (struct out_buffer refbuf)
{
  TEST_VERIFY (!out_buffer_has_failed (&refbuf));
  TEST_VERIFY (IS_ALIGNED (START (&refbuf), short));
  test_empty_array (refbuf);
  test_impossible_array (refbuf);

  /* Success adding two bytes.  */
  {
    struct out_buffer buf = refbuf;
    out_buffer_add_byte (&buf, '@');
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    test_size_1 (buf);
  }
  TEST_VERIFY (memcmp (START (&refbuf), "@\xfd", 2) == 0);
  {
    struct out_buffer buf = refbuf;
    signed char *ptr = out_buffer_get (&buf, signed char);
    TEST_VERIFY_EXIT (ptr != NULL);
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    *ptr = 'A';
    test_size_1 (buf);
  }
  TEST_VERIFY (memcmp (START (&refbuf), "A\xfd", 2) == 0);
  {
    struct out_buffer buf = refbuf;
    char *ptr = out_buffer_get_array (&buf, char, 1);
    TEST_VERIFY_EXIT (ptr != NULL);
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    *ptr = 'B';
    test_size_1 (buf);
  }
  TEST_VERIFY (memcmp (START (&refbuf), "B\xfd", 2) == 0);
  {
    struct out_buffer buf = refbuf;
    unsigned short *ptr = out_buffer_get (&buf, unsigned short);
    TEST_VERIFY_EXIT (ptr != NULL);
    TEST_VERIFY (IS_ALIGNED (ptr, unsigned short));
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    *ptr = htons (0x12f4);
    test_empty (buf);
  }
  TEST_VERIFY (memcmp (START (&refbuf), "\x12\xf4", 2) == 0);
  {
    struct out_buffer buf = refbuf;
    unsigned short *ptr = out_buffer_get_array (&buf, unsigned short, 1);
    TEST_VERIFY_EXIT (ptr != NULL);
    TEST_VERIFY (IS_ALIGNED (ptr, unsigned short));
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    *ptr = htons (0x13f5);
    test_empty (buf);
  }
  TEST_VERIFY (memcmp (START (&refbuf), "\x13\xf5", 2) == 0);
  {
    struct out_buffer buf = refbuf;
    char *ptr = out_buffer_get_array (&buf, char, 2);
    TEST_VERIFY_EXIT (ptr != NULL);
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    memcpy (ptr, "12", 2);
    test_empty (buf);
  }
  TEST_VERIFY (memcmp (START (&refbuf), "12", 2) == 0);
}

static void
test_misaligned (char pad)
{
  enum { SIZE = 23 };
  char *backing = xmalloc (SIZE + 2);
  backing[0] = ~pad;
  backing[SIZE + 1] = pad;
  struct out_buffer refbuf;
  out_buffer_init (&refbuf, backing + 1, SIZE);

  {
    struct out_buffer buf = refbuf;
    short *ptr = out_buffer_get_array (&buf, short, SIZE / sizeof (short));
    TEST_VERIFY_EXIT (ptr != NULL);
    TEST_VERIFY (IS_ALIGNED (ptr, short));
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    for (int i = 0; i < SIZE / sizeof (short); ++i)
      ptr[i] = htons (0xff01 + i);
    TEST_VERIFY (memcmp (ptr,
                         "\xff\x01\xff\x02\xff\x03\xff\x04"
                         "\xff\x05\xff\x06\xff\x07\xff\x08"
                         "\xff\x09\xff\x0a\xff\x0b", 22) == 0);
  }
  {
    struct out_buffer buf = refbuf;
    uint32_t *ptr = out_buffer_get_array
      (&buf, uint32_t, SIZE / sizeof (uint32_t));
    TEST_VERIFY_EXIT (ptr != NULL);
    TEST_VERIFY (IS_ALIGNED (ptr, uint32_t));
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    for (int i = 0; i < SIZE / sizeof (uint32_t); ++i)
      ptr[i] = htonl (0xf1e2d301 + i);
    TEST_VERIFY (memcmp (ptr,
                         "\xf1\xe2\xd3\x01\xf1\xe2\xd3\x02"
                         "\xf1\xe2\xd3\x03\xf1\xe2\xd3\x04"
                         "\xf1\xe2\xd3\x05", 20) == 0);
  }
  {
    struct out_buffer buf = refbuf;
    struct twelve *ptr = out_buffer_get (&buf, struct twelve);
    TEST_VERIFY_EXIT (ptr != NULL);
    TEST_VERIFY (IS_ALIGNED (ptr, struct twelve));
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    ptr->buffer[0] = htonl (0x11223344);
    ptr->buffer[1] = htonl (0x55667788);
    ptr->buffer[2] = htonl (0x99aabbcc);
    TEST_VERIFY (memcmp (ptr,
                         "\x11\x22\x33\x44"
                         "\x55\x66\x77\x88"
                         "\x99\xaa\xbb\xcc", 12) == 0);
  }
  {
    static const double nums[] = { 1, 2 };
    struct out_buffer buf = refbuf;
    double *ptr = out_buffer_get_array (&buf, double, 2);
    TEST_VERIFY_EXIT (ptr != NULL);
    TEST_VERIFY (IS_ALIGNED (ptr, double));
    TEST_VERIFY (!out_buffer_has_failed (&buf));
    ptr[0] = nums[0];
    ptr[1] = nums[1];
    TEST_VERIFY (memcmp (ptr, nums, sizeof (nums)) == 0);
  }

  /* Verify that padding was not overwritten.  */
  TEST_VERIFY (backing[0] == ~pad);
  TEST_VERIFY (backing[SIZE + 1] == pad);
  free (backing);
}

/* Check that overflow during alignment is handled properly.  */
static void
test_large_misaligned (void)
{
  uintptr_t minus1 = -1;
  uintptr_t start = minus1 & ~0xfe;
  struct out_buffer refbuf;
  out_buffer_init (&refbuf, (void *) start, 16);
  TEST_VERIFY (!out_buffer_has_failed (&refbuf));

  struct __attribute__ ((aligned (256))) align256
  {
    int dymmy;
  };

  {
    struct out_buffer buf = refbuf;
    TEST_VERIFY (out_buffer_get (&buf, struct align256) == NULL);
    test_after_failure (buf);
  }
  for (int count = 0; count < 3; ++count)
    {
      struct out_buffer buf = refbuf;
      TEST_VERIFY (out_buffer_get_array (&buf, struct align256, count) == NULL);
      test_after_failure (buf);
    }
}

static struct out_buffer
mkbuffer (void *ptr, size_t len)
{
  struct out_buffer buf;
  out_buffer_init (&buf, ptr, len);
  return buf;
}

static int
do_test (void)
{
  test_empty (mkbuffer (NULL, 0));
  test_empty (mkbuffer ((char *) "", 0));
  test_empty (mkbuffer ((void *) 1, 0));

  {
    char *backing = xmalloc (1);
    struct out_buffer buf;
    out_buffer_init (&buf, backing, 1);
    test_size_1 (buf);
    free (backing);
  }

  {
    char *backing = xmalloc (2);
    struct out_buffer buf;
    out_buffer_init (&buf, backing, 2);
    test_size_2 (buf);
    free (backing);
  }

  test_misaligned (0);
  test_misaligned (0xc7);
  test_misaligned (0xff);

  test_large_misaligned ();

  return 0;
}

#include <support/test-driver.c>
