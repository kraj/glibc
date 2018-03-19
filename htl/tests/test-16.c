/* Test pthread_kill.c.  */

#define _GNU_SOURCE

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <error.h>
#include <errno.h>
#include <hurd/signal.h>

pthread_t testthread;

int i;

void *
test (void *arg)
{
  error_t err;

  printf ("test: %d\n", pthread_self ());

  err = pthread_kill (pthread_self (), SIGINFO);
  if (err)
    error (1, err, "pthread_kill");

  /* To avoid using condition variables in a signal handler.  */
  while (i == 0)
    sched_yield ();

  return 0;
}

static void
handler (int sig)
{
  assert (pthread_equal (pthread_self (), testthread));
  printf ("handler: %d\n", pthread_self ());
  i = 1;
}

int
main (int argc, char **argv)
{
  error_t err;
  struct sigaction sa;
  void *ret;

  printf ("main: %d\n", pthread_self ());

  sa.sa_handler = handler;
  sa.sa_mask = 0;
  sa.sa_flags = 0;

  err = sigaction (SIGINFO, &sa, 0);
  if (err)
    error (1, err, "sigaction");

  err = pthread_create (&testthread, 0, test, 0);
  if (err)
    error (1, err, "pthread_create");

  err = pthread_join (testthread, &ret);
  if (err)
    error (1, err, "pthread_join");

  assert (ret == 0);

  return 0;
}
