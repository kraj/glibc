/* Common definition for qsort testing/benchmark.
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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <support/support.h>
#include <sys/random.h>
#include <time.h>

/* Type of inputs arrays:
   - SORTED:       array already sorted in placed.
   - MOSTLYSORTED: sorted array with 'MOSTLY_SORTED_RATIO * size' elements
		   in random positions set to random values.
   - RANDOM:       all elements in array set to random values.
   - REPEATED:     random array with 'RepeatedRation' elements in random
		   positions set to an unique value.
   - BITONIC:      strictly increasing to up middle then strictly
		   decreasing.  */
typedef enum
{
  SORTED,
  MOSTLYSORTED,
  RANDOM,
  REPEATED,
  BITONIC
} arraytype_t;

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

/* Ratio of total of elements which will randomized.  */
#define MOSTLY_SORTED_RATIO (0.2)

/* Ratio of total of elements which will be repeated.  */
#define REPEATED_RATIO (0.2)

/* Return the index of BASE as interpreted as an array of elements
   of size SIZE.  */
static inline void *
arr (void *base, size_t idx, size_t size)
{
  return (void*)((uintptr_t)base + (idx * size));
}

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
typedef enum
{
  UINT8_CMP_T,
  UINT16_CMP_T,
  UINT32_CMP_T,
  UINT64_CMP_T
} type_cmp_t;

static type_cmp_t
__attribute__((used))
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
__attribute__((used))
uint_t_cmp (const void *a, const void *b, void *arg)
{
  type_cmp_t type = *(type_cmp_t*) arg;
  switch (type)
    {
    case UINT8_CMP_T:  return uint8_t_cmp (a, b);
    case UINT16_CMP_T: return uint16_t_cmp (a, b);
    case UINT64_CMP_T: return uint64_t_cmp (a, b);
    case UINT32_CMP_T:
    default:           return uint32_t_cmp (a, b);
    }
}

static void
seq (void *elem, size_t type_size, uint64_t value)
{
  if (type_size == sizeof (uint8_t))
    {
      uint8_t x = value;
      memcpy (elem, &x, type_size);
    }
  else if (type_size == sizeof (uint16_t))
    {
      uint16_t x = value;
      memcpy (elem, &x, type_size);
    }
  else if (type_size == sizeof (uint32_t))
    {
      uint32_t x = value;
      memcpy (elem, &x, type_size);
    }
  else if (type_size == sizeof (uint64_t))
    memcpy (elem, &value, type_size);
  else
    memset (elem, value, type_size);
}

static void
random_init (void)
{
  unsigned short seed16v[3];
  if (getrandom (seed16v, sizeof seed16v, 0) == -1)
    srand (time (NULL));
  else
    seed48 (seed16v);
}

static uint32_t
random_uniform_distribution (uint32_t min, uint32_t max)
{
  uint32_t ret;
  uint32_t range = max - min;
  /* It assumes the input random number RANDOM range is as larger or equal
     than the RANGE, so the result will either returned or downscaled.  */
  if (range != UINT32_MAX)
    {
      uint32_t urange = range + 1;  /* range can be 0.  */
      uint32_t scaling = UINT32_MAX / urange;
      uint32_t past = urange * scaling;
      do
        ret = lrand48 ();
      while (ret >= past);
      ret /= scaling;
    }
  else
    ret = lrand48 ();
  return ret + min;
}

void
random_buf (void *buf, size_t nbytes)
{
  size_t nw = nbytes / sizeof (uint32_t);
  for (size_t i = 0; i < nw; i++)
    {
      uint32_t r = lrand48 ();
      memcpy (buf, &r, sizeof (uint32_t));
      buf = (void*)((uintptr_t)buf + sizeof (uint32_t));
    }

  size_t nb = nbytes % sizeof (uint32_t);
  if (nb != 0)
    {
      uint32_t r = lrand48 ();
      memcpy (buf, &r, nb);
    }
}

static void *
create_array (size_t nmemb, size_t type_size, arraytype_t type)
{
  size_t size = nmemb * type_size;
  void *array = xmalloc (size);

  switch (type)
    {
    case SORTED:
      for (uint64_t i = 0; i < nmemb; i++)
	seq (arr (array, i, type_size), type_size, i);
      break;

    case MOSTLYSORTED:
      {
        for (uint64_t i = 0; i < nmemb; i++)
	  seq (arr (array, i, type_size), type_size, i);

	/* Change UNSORTED elements (based on MostlySortedRatio ratio)
	   in the sorted array.  */
        size_t unsorted = (size_t)(nmemb * MOSTLY_SORTED_RATIO);
	for (size_t i = 0; i < unsorted; i++)
	  {
	    size_t pos = random_uniform_distribution (0, nmemb - 1);
            random_buf (arr (array, pos, type_size), type_size);
	  }
      }
      break;

    case RANDOM:
      random_buf (array, size);
      break;

    case REPEATED:
      {
        random_buf (array, size);

	void *randelem = xmalloc (type_size);
	random_buf (randelem, type_size);

	/* Repeat REPEATED elements (based on RepeatRatio ratio) in the random
	   array.  */
        size_t repeated = (size_t)(nmemb * REPEATED_RATIO);
	for (size_t i = 0; i < repeated; i++)
	  {
	    size_t pos = random_uniform_distribution (0, nmemb - 1);
	    memcpy (arr (array, pos, type_size), randelem, type_size);
	  }

	free (randelem);
      }
      break;

    case BITONIC:
      {
	uint64_t i;
        for (i = 0; i < nmemb / 2; i++)
	  seq (arr (array, i, type_size), type_size, i);
        for (     ; i < nmemb;     i++)
	  seq (arr (array, i, type_size), type_size, (nmemb - 1) - i);
      }
      break;
    }

  return array;
}

typedef int (*cmpfunc_t)(const void *, const void *);
