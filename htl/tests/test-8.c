#define _GNU_SOURCE

#include <pthread.h>
#include <assert.h>
#include <error.h>
#include <errno.h>

#define THREADS 10

pthread_once_t inc_var_once = PTHREAD_ONCE_INIT;
int var;

void
inc_var (void)
{
  var ++;
}

void *
thr (void *arg)
{
  int i;

  for (i = 0; i < 500; i ++)
    pthread_once (&inc_var_once, inc_var);

  return 0;
}

int
main (int argc, char **argv)
{
  error_t err;
  int i;
  pthread_t tid[THREADS];

  for (i = 0; i < THREADS; i ++)
    {
      err = pthread_create (&tid[i], 0, thr, 0);
      if (err)
	error (1, err, "pthread_create (%d)", i);
    }

  assert (thr (0) == 0);

  for (i = 0; i < THREADS; i ++)
    {
      void *ret;

      err = pthread_join (tid[i], &ret);
      if (err)
	error (1, err, "pthread_join");

      assert (ret == 0);
    }

  assert (var == 1);

  return 0;
}
