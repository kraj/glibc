/* mmap wrapper for early static startup.  Linux version.
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

#ifndef _DL_MMAP_H
#define _DL_MMAP_H

#include <sys/mman.h>
#include <sysdep.h>
#include <mmap_call.h>

static inline void *
_dl_mmap (void *addr, size_t len, int prot, int flags)
{
  long int ret;
#ifdef __NR_mmap2
  ret = MMAP_CALL_INTERNAL (mmap2, addr, len, prot, flags, -1, 0);
#else
  ret = MMAP_CALL_INTERNAL (mmap, addr, len, prot, flags, -1, 0);
#endif
  if (INTERNAL_SYSCALL_ERROR_P (ret))
    return MAP_FAILED;
  return (void *) ret;
}

#endif
