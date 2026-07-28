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

#ifndef _DL_MMAP_H
#define _DL_MMAP_H

/* i386 requires out-of-line implementation because it sets
   I386_USE_SYSENTER to 0 to avoid use the vDSO.  */
void * _dl_mmap (void *addr, size_t len, int prot, int flags)
  attribute_hidden;

#endif
