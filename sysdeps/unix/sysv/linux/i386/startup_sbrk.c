/* Linux/i386 definitions of _startup_sbrk.
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

#include <unistd.h>
#include <startup.h>
#include <errno.h>
#include <sysdep.h>

/* Defined in brk.c.  */
extern void *__curbrk attribute_hidden;

static int
startup_brk (void *addr)
{
  INTERNAL_SYSCALL_DECL (err);
  void *newbrk = (void *) INTERNAL_SYSCALL_CALL (brk, err, addr);
  __curbrk = newbrk;
  if (newbrk < addr)
    _startup_fatal (NULL);
  return 0;
}

/* Extend the process's data space by INCREMENT.  If INCREMENT is negative,
   shrink data space by - INCREMENT.  Return start of new space allocated,
   or call _startup_fatal for errors.  */

void *
_startup_sbrk (intptr_t increment)
{
  void *oldbrk;

  /* Update __curbrk from the kernel's brk value.  That way two separate
     instances of __brk and __sbrk can share the heap, returning
     interleaved pieces of it.  */
  if (__curbrk == NULL)
    if (startup_brk (0) < 0)		/* Initialize the break.  */
      _startup_fatal (NULL);

  if (increment == 0)
    return __curbrk;

  oldbrk = __curbrk;
  if (increment > 0
      ? ((uintptr_t) oldbrk + (uintptr_t) increment < (uintptr_t) oldbrk)
      : ((uintptr_t) oldbrk < (uintptr_t) -increment))
    _startup_fatal (NULL);

  if (startup_brk (oldbrk + increment) < 0)
    _startup_fatal (NULL);

  return oldbrk;
}
