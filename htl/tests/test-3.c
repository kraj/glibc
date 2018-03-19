/* Test the thread attribute get and set methods.  */

#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <assert.h>
#include <errno.h>

int
main (int argc, char *argv[])
{
  error_t err;
  pthread_attr_t attr;

  int i;
  struct sched_param sp;
  void *p;
  size_t sz;

  err = pthread_attr_init (&attr);
  assert_perror (err);

  err = pthread_attr_destroy (&attr);
  assert_perror (err);

  err = pthread_attr_init (&attr);
  assert_perror (err);

#define TEST1(foo, rv, v) \
	err = pthread_attr_get##foo (&attr, rv); \
	assert_perror (err); \
	\
	err = pthread_attr_set##foo (&attr, v); \
	assert_perror (err);

#define TEST(foo, rv, v) TEST1(foo, rv, v)

  TEST(inheritsched, &i, i);
  TEST(schedparam, &sp, &sp);
  TEST(schedpolicy, &i, i);
  TEST(scope, &i, i);
  TEST(stackaddr, &p, p);
  TEST(detachstate, &i, i);
  TEST(guardsize, &sz, sz);
  TEST(stacksize, &sz, sz);

  err = pthread_attr_getstack (&attr, &p, &sz);
  assert_perror (err);

  err = pthread_attr_setstack (&attr, p, sz);
  assert_perror (err);

  return 0;
}
