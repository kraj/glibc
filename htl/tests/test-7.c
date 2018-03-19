#define _GNU_SOURCE

#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>

#define THREADS 10
#define KEYS 400

pthread_key_t key[KEYS];

void *
thr (void *arg)
{
  error_t err;
  int i;

  for (i = 0; i < KEYS; i ++)
    {
      printf ("pthread_getspecific(%d).\n", key[i]);
      assert (pthread_getspecific (key[i]) == NULL);
      printf ("pthread_setspecific(%d, %d).\n", key[i], pthread_self ());
      err = pthread_setspecific (key[i], (void *) pthread_self ());
      printf ("pthread_setspecific(%d, %d) => %d.\n", key[i], pthread_self (), err);
      assert_perror (err);
    }

  return 0;
}

int
main (int argc, char **argv)
{
  error_t err;
  int i;
  pthread_t tid[THREADS];

  void des (void *val)
    {
      assert ((pthread_t) val == pthread_self ());
    }

  assert (pthread_getspecific ((pthread_key_t) 0) == NULL);
  assert (pthread_setspecific ((pthread_key_t) 0, (void *) 0x1) == EINVAL);

  for (i = 0; i < KEYS; i ++)
    err = pthread_key_create (&key[i], des);

  for (i = 0; i < THREADS; i ++)
    {
      err = pthread_create (&tid[i], 0, thr, 0);
      if (err)
	error (1, err, "pthread_create (%d)", i);
    }

  for (i = 0; i < THREADS; i ++)
    {
      void *ret;

      err = pthread_join (tid[i], &ret);
      if (err)
	error (1, err, "pthread_join");

      assert (ret == 0);
    }

  return 0;
}
