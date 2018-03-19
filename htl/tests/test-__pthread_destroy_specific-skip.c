/* Check that __pthread_destroy_specific works correctly if it has to skip
   unused slots.  */

#define _GNU_SOURCE

#include <error.h>
#include <pthread.h>
#include <stdio.h>


#define N_k 42

static volatile int v;

static void
d (void *x)
{
  int *i = (int *) x;

  if (v != *i)
    error (1, 0, "FAILED %d %d", v, *i);
  v += 2;

  printf ("%s %d\n", __FUNCTION__, *i);
  fflush (stdout);
}

static void *
test (void *x)
{
  pthread_key_t k[N_k];
  static int k_v[N_k];

  int err, i;

  for (i = 0; i < N_k; i += 1)
    {
      err = pthread_key_create (&k[i], &d);
      if (err != 0)
        error (1, err, "pthread_key_create %d", i);
    }

  for (i = 0; i < N_k; i += 1)
    {
      k_v[i] = i;
      err = pthread_setspecific (k[i], &k_v[i]);
      if (err != 0)
        error (1, err, "pthread_setspecific %d", i);
    }

  /* Delete every even key.  */
  for (i = 0; i < N_k; i += 2)
    {
      err = pthread_key_delete (k[i]);
      if (err != 0)
        error (1, err, "pthread_key_delete %d", i);
    }

  v = 1;
  pthread_exit (NULL);

  return NULL;
}


int main(void)
{
  pthread_t tid;
  int err;

  err = pthread_create (&tid, 0, test, NULL);
  if (err != 0)
    error (1, err, "pthread_create");

  err = pthread_join(tid, NULL);
  if (err)
    error (1, err, "pthread_join");

  if (v != N_k + 1)
    error (1, 0, "FAILED END %d %d", v, N_k + 1);

  return 0;
}
