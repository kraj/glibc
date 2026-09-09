/* Get the stack bounds of a thread.
   Copyright (C) 2026 Free Software Foundation, Inc.
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

#include <pthreadP.h>

int
pthread_getstack_np (pthread_t threadid, void **stackaddr, size_t *stacksize)
{
  struct pthread *thread = (struct pthread *) threadid;

  if (__glibc_likely (thread->stackblock != NULL))
    {
      /* As per pthread_attr_getstack, the returned size does not include the
	 guard area.  */
      *stacksize = thread->stackblock_size - thread->guardsize;
#if _STACK_GROWS_DOWN
      *stackaddr = (char *) thread->stackblock + thread->guardsize;
#else
      *stackaddr = thread->stackblock;
#endif
      return 0;
    }

  /* The initial thread stack is set up by the kernel.  */
  void *addr;
  size_t size;
  int ret = __pthread_main_stack (&addr, &size);
  if (ret != 0)
    return ret;

#if _STACK_GROWS_DOWN
  *stackaddr = (char *) addr - size;
#else
  *stackaddr = addr;
#endif
  *stacksize = size;
  return 0;
}
