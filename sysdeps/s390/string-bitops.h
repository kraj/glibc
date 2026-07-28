/* Zero byte detection, define whether to use stdbit.h  s390 version.
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

/* s390x supports static-pie, and the libgcc implementation for
   __builtin_clzl/__builtin_ctzl (__clzdi2) accesses extern data (__clz_tab)
   that is not marked as hidden, which creates an additional GOT access.  The
   generic strchrnul is used before self-relocation, so the GOT entry can not
   be resolved yet.  */
#if __ARCH__ > 6
# define HAVE_BITOPTS_WORKING 1
#else
# define HAVE_BITOPTS_WORKING 0
#endif
