#define _GNU_SOURCE

#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <stdio.h>

#define THREADS 500

void *
foo (void *arg)
{
  pthread_mutex_t *mutex = arg;
  pthread_mutex_lock (mutex);
  pthread_mutex_unlock (mutex);
  return mutex;
}

int
main (int argc, char **argv)
{
  int i;
  error_t err;
  pthread_t tid[THREADS];
  pthread_mutex_t mutex[THREADS];

  for (i = 0; i < THREADS; i ++)
    {
      pthread_mutex_init (&mutex[i], 0);
      pthread_mutex_lock (&mutex[i]);
      err = pthread_create (&tid[i], 0, foo, &mutex[i]);
      if (err)
	error (1, err, "pthread_create");
      sched_yield ();
    }

  for (i = THREADS - 1; i >= 0; i --)
    {
      void *ret;
      pthread_mutex_unlock (&mutex[i]);
      err = pthread_join (tid[i], &ret);
      if (err)
	error (1, err, "pthread_join");
      assert (ret == &mutex[i]);
    }

  return 0;
}
