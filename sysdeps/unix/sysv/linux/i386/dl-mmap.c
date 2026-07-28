/* mmap wrapper for early static startup.  Linux/i386 version.
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

/* This mmap call is used to allocate some memory to backup assert() messages
   before TLS setup is done, so it cannot use "call *%gs:SYSINFO_OFFSET"
   during startup in static PIE.  */
#if BUILD_PIE_DEFAULT
# define I386_USE_SYSENTER 0
#endif

#include <sys/mman.h>
#include <dl-mmap.h>
#include <sysdep.h>
#include <mmap_call.h>

void *
_dl_mmap (void *addr, size_t len, int prot, int flags)
{
  long int ret = MMAP_CALL_INTERNAL (mmap2, addr, len, prot, flags, -1, 0);
  if (INTERNAL_SYSCALL_ERROR_P (ret))
    return MAP_FAILED;
  return (void *) ret;
}
