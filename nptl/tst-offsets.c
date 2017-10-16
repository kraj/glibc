/* Check pthread internal offsets.
   Copyright (C) 2017 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <pthreadP.h>
#include <semaphore.h>

#include <pthread-offsets.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

static int
do_test (void)
{
#define TEST_OFFSET(type, member, offset) \
  _Static_assert (offsetof (type, member) == offset, \
		  "offset of " #member " field of " #type " != " \
		  STR (offset))

  /* Check if internal fields in pthread_mutex_t used by static initializers
     have the expected offset.  */
  TEST_OFFSET (pthread_mutex_t, __data.__nusers,
	       __PTHREAD_MUTEX_NUSERS_OFFSET);
  TEST_OFFSET (pthread_mutex_t, __data.__kind,
	       __PTHREAD_MUTEX_KIND_OFFSET);
  TEST_OFFSET (pthread_mutex_t, __data.__spins,
	       __PTHREAD_MUTEX_SPINS_OFFSET);
#if __PTHREAD_MUTEX_LOCK_ELISION
  TEST_OFFSET (pthread_mutex_t, __data.__elision,
	       __PTHREAD_MUTEX_ELISION_OFFSET);
#endif
  TEST_OFFSET (pthread_mutex_t, __data.__list,
	       __PTHREAD_MUTEX_LIST_OFFSET);

  return 0;
}

#include <support/test-driver.c>
