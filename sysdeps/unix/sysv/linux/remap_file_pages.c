/* Remap pages of a shared file mapping.  Linux version.
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

/* The kernel takes PROT and FLAGS as unsigned long.  Unlike a
   syscalls.list wrapper, INLINE_SYSCALL_CALL extends them to the full
   register on ABIs that leave the upper bits of a 32-bit argument
   unspecified.  */
int
__remap_file_pages (void *start, size_t size, int prot, size_t pgoff,
		    int flags)
{
  return INLINE_SYSCALL_CALL (remap_file_pages, start, size, prot, pgoff,
			      flags);
}
weak_alias (__remap_file_pages, remap_file_pages)
