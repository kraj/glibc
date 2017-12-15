#include <stdio.h>
#include <stdlib.h>

#include <support/check.h>

static char *array;
static char *array_end;
static size_t member_size;

static int
compare (const void *a1, const void *b1)
{
  const char *a = a1;
  const char *b = b1;

  if (! (array <= a && a < array_end
	 && array <= b && b < array_end))
    {
      puts ("compare arguments not inside of the array");
      exit (EXIT_FAILURE);
    }
  int ret = b[0] - a[0];
  if (ret)
    return ret;
  if (member_size > 1)
    return b[1] - a[1];
  return 0;
}

static int
test (size_t nmemb, size_t size)
{
  array = malloc (nmemb * size);
  if (array == NULL)
    {
      printf ("%zd x %zd: no memory", nmemb, size);
      return 1;
    }

  array_end = array + nmemb * size;
  member_size = size;

  char *p;
  size_t i;
  size_t bias = random ();
  for (i = 0, p = array; i < nmemb; i++, p += size)
    {
      p[0] = (char) (i + bias);
      if (size > 1)
	p[1] = (char) ((i + bias) >> 8);
    }

  qsort (array, nmemb, size, compare);

  for (i = 0, p = array; i < nmemb - 1; i++, p += size)
    {
      if (p[0] < p[size]
	  || (size > 1 && p[0] == p[size] && p[1] < p[size + 1]))
	{
	  printf ("%zd x %zd: failure at offset %zd\n", nmemb,
		  size, i);
	  free (array);
	  return 1;
	}
    }

  free (array);
  return 0;
}

static int
do_test (void)
{
  test (10000, 1);
  test (200000, 2);
  test (2000000, 3);
  test (2132310, 4);
  test (1202730, 7);
  test (1184710, 8);
  test (272710, 12);
  test (14170, 32);
  test (4170, 320);

  return 0;
}

#include <support/test-driver.c>
