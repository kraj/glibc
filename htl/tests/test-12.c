/* Test concurrency level.  */

#define _GNU_SOURCE

#include <pthread.h>
#include <assert.h>
#include <error.h>
#include <errno.h>

int
main (int argc, char **argv)
{
  int i;
  int err;

  i = pthread_getconcurrency ();
  assert (i == 0);

  err = pthread_setconcurrency (-1);
  assert (err == EINVAL);

  err = pthread_setconcurrency (4);
  assert (err == 0);

  i = pthread_getconcurrency ();
  assert (i == 4);

  return 0;
}
