/* Test signals.  */

#define _GNU_SOURCE

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <error.h>
#include <assert.h>
#include <sys/resource.h>
#include <sys/wait.h>

void *
thr (void *arg)
{
  * (int *)0 = 0;
  return 0;
}

int foobar;

int
main (int argc, char *argv[])
{
  error_t err;
  pid_t child;

  struct rlimit limit;

  limit.rlim_cur = 0;
  limit.rlim_max = 0;

  err = setrlimit (RLIMIT_CORE, &limit);
  if (err)
    error (1, err, "setrlimit");

  child = fork ();
  switch (child)
    {
    case -1:
      error (1, errno, "fork");
      break;

    case 0:
      {
	pthread_t tid;
	void *ret;

	err = pthread_create (&tid, 0, thr, 0);
	if (err)
	  error (1, err, "pthread_create");

	err = pthread_join (tid, &ret);
	assert_perror (err);

	/* Should have never returned.  Our parent expects us to fail
	   thus we succeed and indicate the error.  */
	return 0;
      }

    default:
      {
	pid_t pid;
	int status;

	pid = waitpid (child, &status, 0);
	printf ("pid = %d; child = %d; status = %d\n", pid, child, status);
	assert (pid == child);
	assert (status != 0);
      }
    }

  return 0;
}
