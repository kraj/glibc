/* Measure qsort function.
   Copyright (C) 2018 Free Software Foundation, Inc.
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
#include <bench-timing.h>
#include <errno.h>
#include <getopt.h>
#include <json-lib.h>
#include <stdio.h>
#include <unistd.h>

#include <stdlib/tst-qsort-common.c>

/* Number of elements of determined type to be measured.  */
static const size_t default_nelems[] =
{
  256/sizeof(size_t),       /* 64/128 which covers mostly used element number
			       on GCC build.  */
  32768/sizeof(size_t),	    /* 4096/8192 to fit on a L1 with 32 KB.  */
  262144/sizeof(size_t),    /* 32768/65536 to fit on a L2 with 256 KB.  */
  4194304/sizeof(size_t),   /* 524288/1048576 to fix on a L3 with 4 MB.  */
};

#define OPT_NELEM 10000
#define OPT_SEED  10001
#define CMDLINE_OPTIONS \
  { "nelem", required_argument, NULL, OPT_NELEM }, \
  { "seed", required_argument, NULL, OPT_SEED },

static const size_t max_nelem = 16;
static size_t *elems = NULL;
static size_t nelem = 0;
static uint64_t seed = 0;
static bool seed_set = false;

static void
cmdline_process_function (int c)
{
  switch (c)
    {
      /* Handle the --nelem option to run different sizes than DEFAULT_ELEM.
	 The elements sizes as passed with a ':' as the delimiter, for
	 instance --nelem 32:128:1024 will ran 32, 128, and 1024 elements.  */
      case OPT_NELEM:
        {
	  elems = xmalloc (max_nelem * sizeof (size_t));
	  nelem = 0;

	  char *saveptr;
	  char *token;
	  token = strtok_r (optarg, ":", &saveptr);
	  if (token == NULL)
	    {
	      printf ("error: invalid --nelem value\n");
	      exit (EXIT_FAILURE);
	    }
	  do
	    {
	      if (nelem == max_nelem)
		{
		  printf ("error: invalid --nelem value (max elem)\n");
		  exit (EXIT_FAILURE);
		}
	      elems[nelem++] = atol (token);
	      token = strtok_r (saveptr, ":", &saveptr);
	    } while (token != NULL);
        }
      break;

      /* handle the --seed option to use a different seed than a random one.
	 The SEED used should be a uint64_t number.  */
      case OPT_SEED:
	{
	  uint64_t value = strtoull (optarg, NULL, 0);
	  if (errno == ERANGE || value > UINT64_MAX)
	    {
	      printf ("error: seed should be a value in range of "
		      "[0, UINT64_MAX]\n");
	      exit (EXIT_FAILURE);
	    }
	  seed = value;
	  seed_set = true;
	}
    }
}

#define CMDLINE_PROCESS cmdline_process_function

static int
do_test (void)
{
  random_init ();


  json_ctx_t json_ctx;

  json_init (&json_ctx, 0, stdout);

  json_document_begin (&json_ctx);
  json_attr_string (&json_ctx, "timing_type", TIMING_TYPE);

  json_attr_object_begin (&json_ctx, "functions");
  json_attr_object_begin (&json_ctx, "qsort");
  json_attr_uint (&json_ctx, "seed", seed);

  json_array_begin (&json_ctx, "results");

  const size_t *welem = elems == NULL ? default_nelems : elems;
  const size_t wnelem = elems == NULL ? array_length (default_nelems) : nelem;

  struct test_t
  {
    size_t type_size;
    cmpfunc_t cmpfunc;
  };
  static const struct test_t tests[] =
  {
    { sizeof (uint32_t), uint32_t_cmp },
    { sizeof (uint64_t), uint64_t_cmp },
    /* Test swap with large elements.  */
    { 32,                uint32_t_cmp },
  };

  const size_t inner_loop_iters = 16;
  for (const struct test_t *test = tests; test < array_end (tests); ++test)
    {
      for (const struct array_t *arraytype = arraytypes;
	   arraytype < array_end (arraytypes);
	   ++arraytype)
	{
	  for (int k = 0; k < wnelem; k++)
	    {
	      json_element_object_begin (&json_ctx);

	      size_t arraysize = welem[k] * test->type_size;

	      json_attr_uint (&json_ctx, "nmemb", welem[k]);
	      json_attr_uint (&json_ctx, "type_size", test->type_size);
	      json_attr_string (&json_ctx, "property", arraytype->name);

	      void *base = create_array (welem[k], test->type_size,
					 arraytype->type);
	      void *work = xmalloc (arraysize);

	      timing_t total = 0;

	      for (int n = 0; n < inner_loop_iters; n++)
	        {
		  memcpy (work, base, arraysize);

	          timing_t start, end, diff;
	          TIMING_NOW (start);
	          qsort (work, welem[k], test->type_size, test->cmpfunc);
	          TIMING_NOW (end);

	          TIMING_DIFF (diff, start, end);
	          TIMING_ACCUM (total, diff);
	        }

	     json_attr_uint (&json_ctx, "timings",
			     (double) total / (double) inner_loop_iters);
	     json_element_object_end (&json_ctx);

	     free (base);
	     free (work);
	   }
        }
    }

  json_array_end (&json_ctx);

  json_attr_object_end (&json_ctx);
  json_attr_object_end (&json_ctx);
  json_document_end (&json_ctx);

  return 0;
}

#define TIMEOUT 600
#include <support/test-driver.c>
