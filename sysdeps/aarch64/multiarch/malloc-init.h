/* Definitions for malloc init: aarch64 version.
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

#ifndef _AARCH64_MALLOC_INIT_H
#define _AARCH64_MALLOC_INIT_H

#define MTE_ACTIVE ((GL (dl_aarch64_mte_scope) & MTE_SCOPE_HEAP) != 0	\
		    && (GL (dl_aarch64_mte_mode) != MTE_MODE_DISABLED))

#define ARCH_INIT_MALLOC()			\
  do						\
    {						\
      if (MTE_ACTIVE)				\
	{					\
	  extra_mmap_prot = PROT_MTE;		\
	  __always_fail_morecore = true;	\
	}					\
    }						\
  while (0)

#endif /* _AARCH64_MALLOC_INIT_H */
