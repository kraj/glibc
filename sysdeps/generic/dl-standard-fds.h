/* Re-check the standard file descriptors in the dynamic linker.  Generic.
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

/* Re-run the startup standard-descriptor check after the dynamic
   linker closed a descriptor in the standard range: a secure process
   must not start with a silently closed standard descriptor.  The
   generic version is a no-op: ports without the check in the dynamic
   linker heal the standard descriptors during libc initialization
   instead (the Hurd does from its _hurd_fd_subinit hook, and does not
   define __libc_check_standard_fds for shared builds).  */
static inline void
_dl_recheck_standard_fds (void)
{
}

#endif
