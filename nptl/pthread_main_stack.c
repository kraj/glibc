/* Determine the stack bounds of the initial thread.
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

#include <errno.h>
#include <ldsodefs.h>
#include <pthreadP.h>
#include <procmaps.h>
#include <stdint.h>
#include <sys/resource.h>

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
static int
find_stack_vma (uintptr_t start, uintptr_t end, const char *perm, void *arg)
{
  struct find_stack_args *args = arg;

  if (start <= args->stack_end && args->stack_end < end)
    {
      args->to = end;
      args->found = true;
      return 1;
    }
#if _STACK_GROWS_DOWN
  args->last_to = end;
#endif
  return 0;
}

int
__pthread_main_stack (void **stackaddr, size_t *stacksize)
{
  /* Stack size limit.  */
  struct rlimit rl;

  /* We need the limit of the stack in any case.  */
  if (__getrlimit (RLIMIT_STACK, &rl) != 0)
    return errno;

  /* We consider the main process stack to have ended with the page
     containing __libc_stack_end.  There is stuff below it in the stack
     too, like the program arguments, environment variables and auxv
     info, but we ignore those pages when returning size so that the
     output is consistent when the stack is marked executable due to a
     loaded DSO requiring it.  */
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

  if (__libc_procmaps_iterate (find_stack_vma, &args)
      == procutils_read_error)
    return errno;
  if (!args.found)
    /* No entry was found (there should always be one).  */
    return ENOENT;

  size_t size = rl.rlim_cur - (size_t) (args.to - (uintptr_t) stack_end);

  /* Cut it down to align it to page size since otherwise we risk going
     beyond rlimit when the kernel rounds up the stack extension
     request.  */
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
