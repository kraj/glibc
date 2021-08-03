#include <array_length.h>

static int
do_test (void)
{
  for (int i = 0; i < array_length (dltest); i++)
    if (!process_test_spec (&dltest[i]))
      return 1;
  return 0;
}

#include <support/test-driver.c>
