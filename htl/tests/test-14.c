/* Test pthread_mutex_timedlock.  */

#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <assert.h>
#include <error.h>
#include <errno.h>
#include <sys/time.h>

int
main (int argc, char **argv)
{
  error_t err;
  struct timespec ts;
  pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  struct timeval before, after;
  int diff;

  gettimeofday (&before, 0);
  ts.tv_sec = before.tv_sec + 1;
  ts.tv_nsec = before.tv_usec * 1000;

  printf ("Starting wait @ %d\n", (int) before.tv_sec);

  pthread_mutex_lock (&m);
  /* A default mutex shall dead lock if locked twice.  As such we do
     not need spawn a second thread.  */
  err = pthread_mutex_timedlock (&m, &ts);
  assert (err == ETIMEDOUT);

  gettimeofday (&after, 0);

  printf ("End wait @ %d\n", (int) after.tv_sec);

  diff = after.tv_sec * 1000000 + after.tv_usec
    - before.tv_sec * 1000000 - before.tv_usec;

  if (diff < 900000 || diff > 1100000)
    error (1, EGRATUITOUS, "pthread_mutex_timedlock waited %d us", diff);

  return 0;
}
