/* Memory tagging handling for GNU dynamic linker.  AArch64 version.
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

#ifdef USE_AARCH64_MEMTAG_ABI
#include <ldsodefs.h>
#include <dl-prop.h>

bool
_dl_mte_setup_stack (void)
{
  GL(dl_stack_prot_flags) |= PROT_MTE;
  void *page = PTR_ALIGN_DOWN (__libc_stack_end, GLRO (dl_pagesize));
  return  __mprotect (page, GLRO (dl_pagesize),
		      GL(dl_stack_prot_flags) | PROT_GROWSDOWN) == 0;
}
#endif
