/* Copyright (C) 2004-2026 Free Software Foundation, Inc.
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
#include <procmaps.h>
#include <stdint.h>
#include <stdlib.h>

struct readonly_area_args
{
  uintptr_t from;
  uintptr_t to;
  size_t size;
};

static bool
check_mapping (uintptr_t from, uintptr_t to, const char *perm, void *arg)
{
  struct readonly_area_args *args = arg;

  if (from < args->to && to > args->from)
    {
      /* Found an entry that at least partially covers the area.  */
      if (perm[0] != 'r' || perm[1] != '-')
	return true;

      if (from <= args->from && to >= args->to)
	args->size = 0;
      else if (from <= args->from)
	args->size -= to - args->from;
      else if (to >= args->to)
	args->size -= args->to - from;
      else
	args->size -= to - from;

      if (args->size == 0)
	return true;
    }

  return false;
}

enum readonly_error_type
__readonly_area_fallback (const void *ptr, size_t size)
{
  struct readonly_area_args args =
    {
      .from = (uintptr_t) ptr,
      .to = (uintptr_t) ptr + size,
      .size = size,
    };

  if (__libc_procmaps_iterate (check_mapping, &args) == procutils_read_error)
    {
      /* It is the system administrator's choice to not have /proc
	 available to this process (e.g., because it runs in a chroot
	 environment.  Don't fail in this case.  */
      if (errno == ENOENT
	  /* The kernel has a bug in that a process is denied access
	     to the /proc filesystem if it is set[ug]id.  There has
	     been no willingness to change this in the kernel so
	     far.  */
	  || errno == EACCES)
	return readonly_procfs_inaccessible;
      /* Process has reached the maximum number of open files or another
	 unusual error.  */
      return readonly_procfs_open_fail;
    }

  return args.size == 0 ? readonly_noerror : readonly_area_writable;
}
