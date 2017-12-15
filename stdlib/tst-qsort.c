/* Test case by Paul Eggert <eggert@twinsun.com> */
#include <stdio.h>
#include <stdlib.h>
#include <tst-stack-align.h>

#include <support/check.h>

struct big { char c[4 * 1024]; };

struct big *array;
struct big *array_end;

static int align_check;

static int
compare (void const *a1, void const *b1)
{
  struct big const *a = a1;
  struct big const *b = b1;

  if (!align_check)
    align_check = TEST_STACK_ALIGN () ? -1 : 1;

  TEST_VERIFY_EXIT (array <= a && a < array_end
		    && array <= b && b < array_end);

  return (b->c[0] - a->c[0]) > 0;
}

int
do_test (void)
{
  const size_t sizes[] = { 8, 16, 24, 48, 96, 192, 384 };
  const size_t sizes_len = sizeof (sizes) / sizeof (sizes[0]);

  for (size_t s = 0; s < sizes_len; s++)
    {
      array = (struct big *) malloc (sizes[s] * sizeof *array);
      TEST_VERIFY_EXIT (array != NULL);

      array_end = array + sizes[s];
      for (size_t i = 0; i < sizes[s]; i++)
        array[i].c[0] = i % 128;

      qsort (array, sizes[s], sizeof *array, compare);
      TEST_VERIFY_EXIT (align_check != -1);

      free (array);
    }

  return 0;
}

#include <support/test-driver.c>
