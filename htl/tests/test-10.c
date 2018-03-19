/* Test error checking mutexes.  */

#define _GNU_SOURCE

#include <pthread.h>
#include <assert.h>
#include <error.h>
#include <errno.h>

int
main (int argc, char **argv)
{
  error_t err;
  pthread_mutexattr_t mattr;
  pthread_mutex_t mutex;

  err = pthread_mutexattr_init (&mattr);
  if (err)
    error (1, err, "pthread_mutexattr_init");

  err = pthread_mutexattr_settype (&mattr, PTHREAD_MUTEX_ERRORCHECK);
  if (err)
    error (1, err, "pthread_mutexattr_settype");

  err = pthread_mutex_init (&mutex, &mattr);
  if (err)
    error (1, err, "pthread_mutex_init");

  err = pthread_mutexattr_destroy (&mattr);
  if (err)
    error (1, err, "pthread_mutexattr_destroy");

  err = pthread_mutex_lock (&mutex);
  assert (err == 0);

  err = pthread_mutex_lock (&mutex);
  assert (err == EDEADLK);

  err = pthread_mutex_unlock (&mutex);
  assert (err == 0);

  err = pthread_mutex_unlock (&mutex);
  assert (err == EPERM);

  return 0;
}
