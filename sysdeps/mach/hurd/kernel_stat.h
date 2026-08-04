/* Internal definitions for stat functions.  Hurd version.
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

/* struct stat and struct stat64 never have the same layout: on 32-bit
   ABIs st_ino, st_size, and st_blocks are narrower in struct stat, and
   on 64-bit ABIs the two structures still differ in the amount of
   trailing spare space (see _SPARE_SIZE in bits/stat.h).  */
#define XSTAT_IS_XSTAT64 0
