/* Utility function to parse the Linux /proc/self/maps.
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

#ifndef _PROCMAPS_H
#define _PROCMAPS_H

#include <procutils.h>
#include <stdint.h>

/* Called for each /proc/self/maps entry, with the mapping address range
   [START, END) and PERM pointing to its 4 character permission field.  It
   should return 0 if the iteration should continue, otherwise the iteration
   stops and returns early.  */
typedef int (*procmaps_closure_t) (uintptr_t start, uintptr_t end,
				   const char *perm, void *arg);

/* Open and read /proc/self/maps, and call CLOSURE with argument ARG on
   each parsed entry.  Entries that fail to parse are skipped.  */
enum procutils_read_result_t
__libc_procmaps_iterate (procmaps_closure_t closure, void *arg)
  attribute_hidden;

#endif
