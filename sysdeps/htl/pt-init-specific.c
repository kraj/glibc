/* __pthread_init_specific.  Hurd version.
   Copyright (C) 2002-2025 Free Software Foundation, Inc.
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
   License along with the GNU C Library;  if not, see
   <https://www.gnu.org/licenses/>.  */

#include <pthread.h>
#include <stdlib.h>

#include <pt-internal.h>

error_t
__pthread_init_specific (struct __pthread *thread)
{
  thread->thread_specifics = 0;
  thread->thread_specifics_size = 0;
  memset (&thread->static_thread_specifics, 0,
	  sizeof (thread->static_thread_specifics));
  return 0;
}
