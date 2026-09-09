/* Copyright (C) 2002-2026 Free Software Foundation, Inc.
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

#include <assert.h>
#include <errno.h>
#include <ldsodefs.h>
#include <procmaps.h>
#include <shlib-compat.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include "pthreadP.h"
#include <lowlevellock.h>


struct find_stack_args
{
  uintptr_t stack_end;
  uintptr_t to;
#if _STACK_GROWS_DOWN
  uintptr_t last_to;
#endif
  bool found;
};

/* Check whether the mapping address range contains ARGS->stack_end.  */
static bool
find_stack_vma (uintptr_t start, uintptr_t end, const char *perm, void *arg)
{
  struct find_stack_args *args = arg;

  if (start <= args->stack_end && args->stack_end < end)
    {
      args->to = end;
      args->found = true;
      return true;
    }
#if _STACK_GROWS_DOWN
  args->last_to = end;
#endif
  return false;
}

static int
pthread_main_stack (void **stackaddr, size_t *stacksize)
{
  /* Stack size limit.  */
  struct rlimit rl;

  /* We need the limit of the stack in any case.  */
  if (__getrlimit (RLIMIT_STACK, &rl) != 0)
    return errno;

  /* We consider the main process stack to have ended with the page
     containing __libc_stack_end.  There is stuff below it in the stack too,
     like the program arguments, environment variables and auxv info, but we
     ignore those pages when returning size so that the output is consistent
     when the stack is marked executable due to a loaded DSO requiring it.  */
  void *stack_end = (void *) ((uintptr_t) __libc_stack_end
			      & -(uintptr_t) GLRO(dl_pagesize));
#if _STACK_GROWS_DOWN
  stack_end += GLRO(dl_pagesize);
#endif
  /* The safest way to get the top of the stack is to read
     /proc/self/maps and locate the line into which __libc_stack_end
     falls.  */
  struct find_stack_args args =
    {
      .stack_end = (uintptr_t) __libc_stack_end,
    };

  if (__libc_procmaps_iterate (find_stack_vma, &args) == procutils_read_error)
    return errno;
  if (!args.found)
    /* No entry was found (there should always be one).  */
    return ENOENT;

  size_t size = rl.rlim_cur - (size_t) (args.to - (uintptr_t) stack_end);

  /* Cut it down to align it to page size since otherwise we risk going beyond
     rlimit when the kernel rounds up the stack extension request.  */
  size &= -(uintptr_t) GLRO(dl_pagesize);
#if _STACK_GROWS_DOWN
  /* The limit might be too high.  */
  if (size > (uintptr_t) stack_end - args.last_to)
    size = (uintptr_t) stack_end - args.last_to;
#else
  /* The limit might be too low.  */
  if (size < args.to - (uintptr_t) stack_end)
    size = args.to - (uintptr_t) stack_end;
#endif

  *stackaddr = stack_end;
  *stacksize = size;
  return 0;
}


int
__pthread_getattr_np (pthread_t thread_id, pthread_attr_t *attr)
{
  struct pthread *thread = (struct pthread *) thread_id;

  /* Prepare the new thread attribute.  */
  int ret = __pthread_attr_init (attr);
  if (ret != 0)
    return ret;

  struct pthread_attr *iattr = (struct pthread_attr *) attr;

  lll_lock (thread->lock, LLL_PRIVATE);

  /* The thread library is responsible for keeping the values in the
     thread descriptor up-to-date in case the user changes them.  */
  memcpy (&iattr->schedparam, &thread->schedparam,
	  sizeof (struct sched_param));
  iattr->schedpolicy = thread->schedpolicy;

  /* Clear the flags work.  */
  iattr->flags = thread->flags;

  /* The thread might be detached by now.  */
  if (atomic_load_relaxed (&thread->joinstate) == THREAD_STATE_DETACHED)
    iattr->flags |= ATTR_FLAG_DETACHSTATE;

  /* This is the guardsize after adjusting it.  */
  iattr->guardsize = thread->reported_guardsize;

  /* The sizes are subject to alignment.  */
  if (__glibc_likely (thread->stackblock != NULL))
    {
      /* The stack size reported to the user should not include the
	 guard size.  */
      iattr->stacksize = thread->stackblock_size - thread->guardsize;
#if _STACK_GROWS_DOWN
      iattr->stackaddr = (char *) thread->stackblock
			 + thread->stackblock_size;
#else
      iattr->stackaddr = (char *) thread->stackblock;
#endif
    }
  else
    /* No stack information available.  This must be for the initial thread.  */
    ret = pthread_main_stack (&iattr->stackaddr, &iattr->stacksize);

  iattr->flags |= ATTR_FLAG_STACKADDR;

  if (ret == 0)
    {
      size_t size = 16;
      cpu_set_t *cpuset = NULL;

      do
	{
	  size <<= 1;

	  void *newp = realloc (cpuset, size);
	  if (newp == NULL)
	    {
	      ret = ENOMEM;
	      break;
	    }
	  cpuset = (cpu_set_t *) newp;

	  ret = __pthread_getaffinity_np (thread_id, size, cpuset);
	}
      /* Pick some ridiculous upper limit.  Is 8 million CPUs enough?  */
      while (ret == EINVAL && size < 1024 * 1024);

      if (ret == 0)
	ret = __pthread_attr_setaffinity_np (attr, size, cpuset);
      else if (ret == ENOSYS)
	/* There is no such functionality.  */
	ret = 0;
      free (cpuset);
    }

  lll_unlock (thread->lock, LLL_PRIVATE);

  if (ret != 0)
    __pthread_attr_destroy (attr);

  return ret;
}
versioned_symbol (libc, __pthread_getattr_np, pthread_getattr_np, GLIBC_2_32);

#if SHLIB_COMPAT (libc, GLIBC_2_2_3, GLIBC_2_32)
strong_alias (__pthread_getattr_np, __pthread_getattr_np_alias)
compat_symbol (libc, __pthread_getattr_np_alias,
	       pthread_getattr_np, GLIBC_2_2_3);
#endif
