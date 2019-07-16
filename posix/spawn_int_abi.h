/* Internal ABI specific for posix_spawn functionality.  Generic version.
   Copyright (C) 2019 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#ifndef _SPAWN_INT_ABI_H
#define _SPAWN_INT_ABI_H

/* The closefrom file actions requires either a syscall or an arch-specific
   way to interact over all file descriptors and act uppon them (such
   /proc/self/fd on Linux).  */
#define __SPAWN_SUPPOR_CLOSEFROM 0

#endif /* _SPAWN_INT_H */
