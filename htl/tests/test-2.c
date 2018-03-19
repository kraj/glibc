/* Test detachability.  */
#define _GNU_SOURCE

#include <pthread.h>
#include <assert.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>

void *
thread (void *arg)
{
  while (1)
    sched_yield ();
}

int
main (int argc, char **argv)
{
  int err;
  pthread_t tid;
  void *ret;

  err = pthread_create (&tid, 0, thread, 0);
  if (err)
    error (1, err, "pthread_create");

  err = pthread_detach (tid);
  if (err)
    error (1, err, "pthread_detach");

  err = pthread_detach (tid);
  assert (err == EINVAL);

  err = pthread_join (tid, &ret);
  assert (err == EINVAL);

  return 0;
}
