/* Run-time dynamic linker data structures for x86 loaded ELF shared objects.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#ifndef	_LDSODEFS_H

/* Get the real definitions.  */
#include_next <ldsodefs.h>

#if defined ENABLE_CET && defined N_ && !IS_IN (ldconfig)
# include <errno.h>

# define DL_OPEN_CHECK dl_cet_open_check

static inline void __attribute__ ((unused))
dl_cet_open_check (struct link_map *l)
{
  int res;

  /* Check IBT and SHSTK when called from dlopen.  */
  if ((GL(dl_x86_feature_1) & GNU_PROPERTY_X86_FEATURE_1_IBT)
      && !(l->l_cet & lc_ibt))
    {
      /* If IBT is enabled in executable and IBT isn't enabled in
	 this shared object, put all executable PT_LOAD segments in
	 legacy code page bitmap.  */
      /* FIXME: Mark legacy region.  */
      res = -EINVAL;
      goto cet_check_failure;
    }

  /* If SHSTK is enabled in executable and SHSTK isn't enabled in
     this shared object, we can't load this shared object.  */
  if ((GL(dl_x86_feature_1) & GNU_PROPERTY_X86_FEATURE_1_SHSTK)
      && !(l->l_cet & lc_shstk))
    {
      res = -EINVAL;

cet_check_failure:
      _dl_signal_error (-res, "dlopen", NULL,
			N_("dl_cet_open_check failed"));
    }
}
#endif

#endif /* ldsodefs.h */
