/* qsort(_r) generic tests.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <errno.h>
#include <stdbool.h>

#include <support/check.h>
#include <support/support.h>
#include <support/support_random.h>
#include <support/test-driver.h>

/* Functions used to check qsort.  */
static int
uint8_t_cmp (const void *a, const void *b)
{
  uint8_t ia = *(uint8_t*)a;
  uint8_t ib = *(uint8_t*)b;
  return (ia > ib) - (ia < ib);
}

static int
uint16_t_cmp (const void *a, const void *b)
{
  uint16_t ia = *(uint16_t*)a;
  uint16_t ib = *(uint16_t*)b;
  return (ia > ib) - (ia < ib);
}

static int
uint32_t_cmp (const void *a, const void *b)
{
  uint32_t ia = *(uint32_t*)a;
  uint32_t ib = *(uint32_t*)b;
  return (ia > ib) - (ia < ib);
}

static int
uint64_t_cmp (const void *a, const void *b)
{
  uint64_t ia = *(uint64_t*)a;
  uint64_t ib = *(uint64_t*)b;
  return (ia > ib) - (ia < ib);
}

/* Function used to check qsort_r.  */

enum type_cmp_t
{
  UINT8_CMP_T  = 0,
  UINT16_CMP_T = 1,
  UINT32_CMP_T = 2,
  UINT64_CMP_T = 3,
};

static enum type_cmp_t
uint_t_cmp_type (size_t sz)
{
  switch (sz)
    {
      case sizeof (uint8_t):  return UINT8_CMP_T;
      case sizeof (uint16_t): return UINT16_CMP_T;
      case sizeof (uint64_t): return UINT64_CMP_T;
      case sizeof (uint32_t):
      default:                return UINT32_CMP_T;
    }
}

static int
uint_t_cmp (const void *a, const void *b, void *arg)
{
  enum type_cmp_t type = *(enum type_cmp_t*) arg;
  switch (type)
    {
    case UINT8_CMP_T:  return uint8_t_cmp (a, b);
    case UINT16_CMP_T: return uint16_t_cmp (a, b);
    case UINT64_CMP_T: return uint64_t_cmp (a, b);
    case UINT32_CMP_T:
    default:           return uint32_t_cmp (a, b);
    }
}

static support_random_state rand_state;

static void *
create_array (size_t nmemb, size_t type_size)
{
  size_t size = nmemb * type_size;
  uint8_t *array = xmalloc (size);
  support_random_buf (&rand_state, array, size);
  return array;
}

typedef int (*cmpfunc_t)(const void *, const void *);

static void
check_array (void *array, size_t nmemb, size_t type_size,
	     cmpfunc_t cmpfunc)
{
  for (size_t i = 1; i < nmemb; i++)
    {
      void *array_i   = (void*)((uintptr_t)array + i * type_size);
      void *array_i_1 = (void*)((uintptr_t)array + (i-1) * type_size);
      int ret;
      TEST_VERIFY ((ret = cmpfunc (array_i, array_i_1)) >= 0);
      if (ret < 0)
	break;
    }
}

static uint32_t seed;
static bool seed_set = false;

#define OPT_SEED 10000
#define CMDLINE_OPTIONS \
  { "seed", required_argument, NULL, OPT_SEED },

static void __attribute__ ((used))
cmdline_process_function (int c)
{
  switch (c)
    {
      case OPT_SEED:
	{
	  unsigned long int value = strtoul (optarg, NULL, 0);
	  if (errno == ERANGE || value > UINT32_MAX)
	    {
	      printf ("error: seed should be a value in range of "
		      "[0, UINT32_MAX]\n");
	      exit (EXIT_FAILURE);
	    }
	  seed = value;
          seed_set = true;
	}
      break;
    }
}

#define CMDLINE_PROCESS cmdline_process_function


static int
do_test (void)
{
  if (test_verbose > 0)
    printf ("info: seed=0x%08x\n", seed);
  if (seed_set)
    support_random_seed (&rand_state, seed);
  else
    support_random_rseed (&rand_state);

  const size_t elem[] = { 0, 1, 64, 128, 4096, 16384, 262144 };
  const size_t nelem = sizeof (elem) / sizeof (elem[0]);

  struct test_t
    {
      size_t type_size;
      cmpfunc_t cmpfunc;
    }
  tests[] =
    {
      { sizeof (uint8_t),  uint8_t_cmp },
      { sizeof (uint16_t), uint16_t_cmp },
      { sizeof (uint32_t), uint32_t_cmp },
      { sizeof (uint64_t), uint64_t_cmp },
      /* Test swap with large elements.  */
      { 32,                uint32_t_cmp },
    };
  size_t ntests = sizeof (tests) / sizeof (tests[0]);

  for (size_t i = 0; i < ntests; i++)
    {
      size_t ts = tests[i].type_size;
      if (test_verbose > 0)
        printf ("info: testing qsort with type_size=%zu\n", ts);
      for (size_t n = 0; n < nelem; n++)
	{
	  size_t nmemb = elem[n];
	  if (test_verbose > 0)
            printf ("  nmemb=%zu, total size=%zu\n", nmemb, nmemb * ts);

	  void *array = create_array (nmemb, ts);

	  qsort (array, nmemb, ts, tests[i].cmpfunc);

	  check_array (array, nmemb, ts, tests[i].cmpfunc);

	  free (array);
	}
    }

  for (size_t i = 0; i < ntests; i++)
    {
      size_t ts = tests[i].type_size;
      if (test_verbose > 0)
        printf ("info: testing qsort_r type_size=%zu\n", ts);
      for (size_t n = 0; n < nelem; n++)
	{
	  size_t nmemb = elem[n];
	  if (test_verbose > 0)
            printf ("  nmemb=%zu, total size=%zu\n", nmemb, nmemb * ts);

	  void *array = create_array (nmemb, ts);

	  enum type_cmp_t type = uint_t_cmp_type (ts);
	  qsort_r (array, nmemb, ts, uint_t_cmp, &type);

	  check_array (array, nmemb, ts, tests[i].cmpfunc);

	  free (array);
	}
    }

  return 0;
}

#include <support/test-driver.c>
