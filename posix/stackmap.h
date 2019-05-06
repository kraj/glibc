/* Functions to create stack mappings for helper processes.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

#ifndef _STACKMAP_H
#define _STACKMAP_H

#include <unistd.h>
#include <sys/mman.h>
#include <ldsodefs.h>
#include <stdbool.h>

static inline int
stack_prot (void)
{
  return (PROT_READ | PROT_WRITE
	  | ((GL(dl_stack_flags) & PF_X) ? PROT_EXEC : 0));
}

static inline size_t
stack_guard_size (void)
{
 return GLRO (dl_pagesize);
}

/* Return a aligning mask based on system pagesize.  */
static inline size_t
stack_pagesize_m1_mask (void)
{
  size_t pagesize_m1 = __getpagesize () - 1;
  return ~pagesize_m1;
}

/* Return the guard page position on memory segment MEM with total size SIZE
   and with a guard page of size GUARDIZE.  */
static inline void *
stack_guard_position (void *mem, size_t size, size_t guardsize)
{
#ifdef NEED_SEPARATE_REGISTER_STACK
  return mem + (((size - guardsize) / 2) & stack_pagesize_m1_mask ());
#elif _STACK_GROWS_DOWN
  return mem;
#elif _STACK_GROWS_UP
  return (void *) (((uintptr_t)(mem + size)- guardsize)
		   & stack_pagesize_m1_mask ());
#endif
}

/* Setup the expected stack memory protection value (based on stack_prot)
   for the memory segment MEM with size SIZE based on the guard page
   GUARD with size GUARDSIZE.  The memory segment is expected to be allocated
   with PROT_NOTE.  */
static inline bool
stack_setup_prot (char *mem, size_t size, char *guard, size_t guardsize)
{
  const int prot = stack_prot ();

  char *guardend = guard + guardsize;
#if _STACK_GROWS_DOWN && !defined(NEED_SEPARATE_REGISTER_STACK)
  /* As defined at guard_position, for architectures with downward stack
     the guard page is always at start of the allocated area.  */
  if (__mprotect (guardend, size - guardsize, prot) != 0)
    return false;
#else
  size_t mprots1 = (uintptr_t) guard - (uintptr_t) mem;
  if (__mprotect (mem, mprots1, prot) != 0)
    return false;
  size_t mprots2 = ((uintptr_t) mem + size) - (uintptr_t) guardend;
  if (__mprotect (guardend, mprots2, prot) != 0)
    return false;
#endif
  return true;
}

/* Allocated a memory segment with size SIZE plus GUARSIZE with mmap and
   setup the expected protection for both a guard page and the stack
   itself.  */
static inline void *
stack_allocate (size_t size, size_t guardsize)
{
  const int prot = stack_prot ();

  /* If a guard page is required, avoid committing memory by first
     allocate with PROT_NONE and then reserve with required permission
     excluding the guard page.  */
  void *mem = __mmap (NULL, size, (guardsize == 0) ? prot : PROT_NONE,
		      MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
  if (guardsize)
    {
      void *guard = stack_guard_position (mem, size, guardsize);
      if (!stack_setup_prot (mem, size, guard, guardsize))
	{
	  __munmap (mem, size);
	  return MAP_FAILED;
	}
    }

  return mem;
}

#endif /* _STACKMAP_H  */
