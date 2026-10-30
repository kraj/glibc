/* Test and measure memrchr functions.
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
#include <support/check.h>
#include <support/xunistd.h>

#define TEST_MAIN
#define TEST_NAME "memrchr"
#include "test-string.h"
#if __WORDSIZE == 64
typedef char *(*proto_t) (const char *, int, size_t);

IMPL (memrchr, 1)

/* Also check the generic implementation.  */
#  undef weak_alias
#  define weak_alias(a, b)
#  define MEMRCHR __memrchr_default
#  include "string/memrchr.c"
IMPL (__memrchr_default, 1)

static void
do_one_test (impl_t *impl, const char *s, int c, size_t n, char *exp_res)
{
  char *res = CALL (impl, s, c, n);
  if (res != exp_res)
    {
      error (0, 0, "Wrong result in function %s %p %p", impl->name, res,
	     exp_res);
      ret = 1;
      return;
    }
}

static void
do_large_tests (size_t large_size)
{
  void *large_buf;
  char *test_buf;
  char seek_char = '\n';
  const size_t tests[] = { 0,  1,  31,	32,  33,  63,  64,  65, 95,
			   96, 97, 127, 128, 129, 159, 160, 161 };
  size_t i, j, ntests = array_length (tests);
  large_size = (large_size + page_size - 1) & (-page_size);
  large_buf = mmap (NULL, large_size + page_size * 3, PROT_READ | PROT_WRITE,
		    MAP_PRIVATE | MAP_ANON | MAP_NORESERVE, -1, 0);

  if (large_buf == MAP_FAILED)
    FAIL_UNSUPPORTED ("Unable to allocate large_buf: %m");

  xmprotect (large_buf, page_size, PROT_NONE);
  test_buf = (char *) large_buf + page_size;
  xmprotect (test_buf + large_size + page_size, page_size, PROT_NONE);

  for (i = 0; i < ntests; ++i)
    {
      size_t align = tests[i];
      for (j = 1; j < ntests; ++j)
	{
	  size_t len = tests[j];
	  test_buf[align + large_size - len] = seek_char;
	  FOR_EACH_IMPL (impl, 0)
	    do_one_test (impl, test_buf + align, seek_char, large_size,
			 test_buf + align + large_size - len);
	  test_buf[align + large_size - len] = 0;
	}
    }
  xmunmap (large_buf, large_size + page_size * 3);
}
#endif

int
test_main (void)
{
#if __WORDSIZE != 64
  FAIL_UNSUPPORTED ("The test requires a VMA larger than 32 bits");
#else
  test_init ();

  printf ("%20s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

  do_large_tests (UINT64_C (1) << 32);
  do_large_tests ((UINT64_C (1) << 32) + page_size);
#endif
  return ret;
}

#include <support/test-driver.c>
