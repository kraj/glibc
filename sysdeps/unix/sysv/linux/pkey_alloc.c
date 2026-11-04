/* Allocate a memory protection key.  Linux version.
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

#include <sys/mman.h>
#include <sysdep.h>

int
__pkey_alloc (unsigned int flags, unsigned int access_restrictions)
{
  return INLINE_SYSCALL_CALL (pkey_alloc, flags, access_restrictions);
}
libc_hidden_def (__pkey_alloc)
weak_alias (__pkey_alloc, pkey_alloc)
