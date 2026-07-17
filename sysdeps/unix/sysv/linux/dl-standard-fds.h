/* Re-check the standard file descriptors in the dynamic linker.  Linux.
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

#ifndef _DL_STANDARD_FDS_H
#define _DL_STANDARD_FDS_H

#include <unistd.h>

/* Linux runs the startup standard-descriptor check inside the dynamic
   linker (dl-sysdep.c), so the re-check after closing a program
   descriptor runs there as well.  */
static inline void
_dl_recheck_standard_fds (void)
{
  __libc_check_standard_fds ();
}

#endif
