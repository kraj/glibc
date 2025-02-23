/* Copyright (C) 2002-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


static int
do_test (void)
{
  puts ("We expect no limits");
  /* We have no fixed limit on the number of threads.  Make sure the
     headers tell the right story.  */
#ifdef PTHREAD_THREADS_MAX
  printf ("Header report maximum number of threads = %lu\n",
	  (unsigned long int) PTHREAD_THREADS_MAX);
  return 1;
#else
  long int r = sysconf (_SC_THREAD_THREADS_MAX);
  if (r != -1)
    {
      printf ("sysconf(_SC_THREAD_THREADS_MAX) return %ld\n", r);
      return 1;
    }
#endif

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
