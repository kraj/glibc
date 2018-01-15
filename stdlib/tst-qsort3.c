/* qsorte if (type_size == sizeof (uint32_t))                                  
   _r) generic tests.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#include <array_length.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <support/check.h>
#include <support/support.h>
#include <support/test-driver.h>

#include <stdlib/tst-qsort-common.c>

/* Check if ARRAY of total NMEMB element of size SIZE is sorted
   based on CMPFUNC.  */
static void
check_array (void *array, size_t nmemb, size_t type_size,
	     cmpfunc_t cmpfunc)
{
  for (size_t i = 1; i < nmemb; i++)
    {
      int ret = cmpfunc (arr (array, i,   type_size),
			 arr (array, i-1, type_size));
      TEST_VERIFY_EXIT (ret >= 0);
    }
}

static void
check_qsort (size_t nelem, size_t type_size, arraytype_t type,
	     cmpfunc_t cmpfunc)
{
  void *array = create_array (nelem, type_size, type);

  qsort (array, nelem, type_size, cmpfunc);

  check_array (array, nelem, type_size, cmpfunc);

  free (array);
}

static void
check_qsort_r (size_t nelem, size_t type_size, arraytype_t type,
	       cmpfunc_t cmpfunc)
{
  void *array = create_array (nelem, type_size, type);

  type_cmp_t typecmp = uint_t_cmp_type (type_size);
  qsort_r (array, nelem, type_size, uint_t_cmp, &typecmp);

  check_array (array, nelem, type_size, cmpfunc);

  free (array);
}

static int
do_test (void)
{
  random_init ();

  struct test_t
    {
      size_t type_size;
      cmpfunc_t cmpfunc;
    };
  static const struct test_t tests[] =
    {
      { sizeof (uint8_t),  uint8_t_cmp },
      { sizeof (uint16_t), uint16_t_cmp },
      { sizeof (uint32_t), uint32_t_cmp },
      { sizeof (uint64_t), uint64_t_cmp },
      /* Test swap with large elements.  */
      { 32,                uint32_t_cmp },
    };

  struct array_t
    {
      arraytype_t type;
      const char *name;
     };
  static const struct array_t arraytypes[] =
    {
      { SORTED,       "SORTED" },
      { MOSTLYSORTED, "MOSTLYSORTED" },
      { RANDOM,       "RANDOM" },
      { REPEATED,     "REPEATED" },
      { BITONIC,      "BITONIC" },
    };

  static const size_t nelems[] =
    { 0, 1, 16, 32, 64, 128, 256, 4096, 16384, 262144 };

  for (const struct test_t *test = tests; test < array_end (tests); ++test)
    {
      if (test_verbose > 0)
	printf ("info: testing qsort with type_size=%zu\n", test->type_size);
      for (const struct array_t *arraytype = arraytypes;
	   arraytype < array_end (arraytypes);
	   ++arraytype)
	{
	  if (test_verbose > 0)
            printf ("  distribution=%s\n", arraytype->name);
	  for (const size_t *nelem = nelems;
	       nelem < array_end (nelems);
	       ++nelem)
	    {
	      if (test_verbose > 0)
		printf ("  i  nelem=%zu, total size=%zu\n", *nelem,
			*nelem * test->type_size);

	      check_qsort (*nelem, test->type_size, arraytype->type,
			   test->cmpfunc);
	      check_qsort_r (*nelem, test->type_size, arraytype->type,
			   test->cmpfunc);
	   }
	}
    }

  return 0;
}

#include <support/test-driver.c>
