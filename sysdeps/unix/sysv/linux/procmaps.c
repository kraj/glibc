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

#include <procmaps.h>
#include <stdlib.h>

struct parse_line_args
{
  procmaps_closure_t closure;
  void *arg;
};

/* Parse the leading "<start>-<end> <perms>" fields of a /proc/self/maps
   line and pass them to the user provided closure.  */
static int
parse_line (const char *line, void *arg)
{
  struct parse_line_args *args = arg;

  char *p;
  uintptr_t start = strtoul (line, &p, 16);
  if (p == line || *p++ != '-')
    return 0;

  char *q;
  uintptr_t end = strtoul (p, &q, 16);
  if (q == p || *q++ != ' ')
    return 0;

  return args->closure (start, end, q, args->arg);
}

enum
{
  /* Each /proc/self/maps line starts with "<start>-<end> <perms> ",
     where START and END are hexadecimal numbers and PERMS is a 4
     character string.  parse_line only parses these leading fields and
     the reader passes longer lines truncated, discarding the rest of
     the line (such as the pathname).  */
  minimum_proc_maps_size = 2 * (2 * sizeof (uintptr_t)) + sizeof ("- rwxp"),
};

enum procutils_read_result_t
__libc_procmaps_iterate (procmaps_closure_t closure, void *arg)
{
  struct parse_line_args args = { .closure = closure, .arg = arg };

  /* A buffer larger than the minimum required size reduces the number
     of read syscalls.  */
  char buffer[2048];
  _Static_assert (sizeof buffer >= minimum_proc_maps_size,
		  "sizeof buffer < minimum_proc_maps_size");
  return __libc_procutils_read_file ("/proc/self/maps", buffer,
				     sizeof buffer, parse_line, &args);
}
