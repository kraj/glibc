/* AArch64 MTE support.
   Copyright (C) 2026 Free Software Foundation, Inc.

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

#ifndef _DL_MTE_H
#define _DL_MTE_H

#ifndef PR_SET_TAGGED_ADDR_CTRL
# define PR_SET_TAGGED_ADDR_CTRL  55
# define PR_MTE_TAG_SHIFT         3
# define PR_TAGGED_ADDR_ENABLE    (1UL << 0)
# define PR_MTE_TCF_SYNC          (1UL << 1)
# define PR_MTE_TCF_ASYNC         (1UL << 2)
#endif

#ifndef USE_AARCH64_MEMTAG_ABI
static __always_inline bool _dl_mte_setup_stack (void) { return false; };
#else
extern bool _dl_mte_setup_stack (void) attribute_hidden;
#endif

#endif
