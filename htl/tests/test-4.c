/* Test the stack guard.  */

#define _GNU_SOURCE

#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

size_t stacksize;

void *
thr (void *arg)
{
  int i;
  char *foo;

  foo = alloca (3 * stacksize / 4);
  for (i = 0; i < sizeof foo; i ++)
    foo[i] = -1;

  return (void *) 1;
}

int
main (int argc, char *argv[])
{
  error_t err;
  pid_t child;

  child = fork ();
  switch (child)
    {
    case -1:
      error (1, errno, "fork");
      break;

    case 0:
      {
	pthread_attr_t attr;
	pthread_t tid;
	void *ret;

	err = pthread_attr_init (&attr);
	assert_perror (err);

	err = pthread_attr_getstacksize (&attr, &stacksize);
	assert_perror (err);

	err = pthread_attr_setguardsize (&attr, stacksize / 2);
	if (err == ENOTSUP)
	  {
	    printf ("Stack guard attribute not supported.\n");
	    return 1;
	  }
	assert_perror (err);

	err = pthread_create (&tid, &attr, thr, 0);
	assert_perror (err);

	err = pthread_attr_destroy (&attr);
	assert_perror (err);

	err = pthread_join (tid, &ret);
	/* Should never be successful.  */
	printf ("Thread did not segfault!?!\n");
	assert_perror (err);
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
