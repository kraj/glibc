/* The ld.so --program-fd option.  Linux version.
   Copyright (C) 2026 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#ifndef _DL_PROGRAM_FD_H
#define _DL_PROGRAM_FD_H

#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <ldsodefs.h>
#include <not-cancel.h>

/* The option's entry in the ld.so --help output.  */
#define DL_PROGRAM_FD_HELP \
  "  --program-fd FD       load the executable from the inherited file\n" \
  "                        descriptor FD; EXECUTABLE-FILE only names it\n"

/* If ARGV[1] is the --program-fd option with its argument in ARGV[2], store
   the descriptor in *FD and return true.  Otherwise return false and leave
   *FD unchanged.
   Exit with a diagnostic naming LD_SO_NAME if the argument is not an open
   descriptor.  */
static inline bool
_dl_program_fd_option (const char *ld_so_name, int argc, char **argv,
		       int *fd)
{
  if (argc <= 2 || strcmp (argv[1], "--program-fd") != 0)
    return false;

  /* Load the program from an inherited descriptor, like AT_EXECFD does; the
     program name argument only names it.  */
  const char *arg = argv[2];
  char *endp;
  uint64_t value = _dl_strtoul (arg, &endp);
  if (arg[0] < '0' || arg[0] > '9' || *endp != '\0'
      || (arg[0] == '0' && arg[1] != '\0') || value > INT_MAX)
    _dl_fatal_printf ("%s: invalid descriptor '%s' given to"
		      " --program-fd\n", ld_so_name, arg);

  /* Unlike AT_EXECFD, a descriptor named on the command line need not be
     open.  Also, its file position should not matter.  */
  if (__fcntl64_nocancel (value, F_GETFD) == -1)
    _dl_fatal_printf ("%s: cannot load main program from descriptor"
		      " %s: descriptor is not open\n", ld_so_name, arg);

  *fd = value;
  return true;
}

#endif
