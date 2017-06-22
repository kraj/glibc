/* Linux/x86 CET initializers function.
   Copyright (C) 2017 Free Software Foundation, Inc.

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

#ifdef SHARED
# include <ldsodefs.h>

# ifndef LINKAGE
#  define LINKAGE
# endif

LINKAGE
void
_dl_cet_init (struct link_map *main_map, int argc, char **argv, char **env)
{
  /* Check if IBT is enabled in executable.  */
  bool enable_ibt
    = ((GL(dl_x86_feature_1) & GNU_PROPERTY_X86_FEATURE_1_IBT)
       && (main_map->l_cet & lc_ibt));

  /* Check if SHSTK is enabled in executable.  */
  bool enable_shstk
    = ((GL(dl_x86_feature_1) & GNU_PROPERTY_X86_FEATURE_1_SHSTK)
       && (main_map->l_cet & lc_shstk));

  if (enable_ibt || enable_shstk)
    {
      unsigned int i;
      struct link_map *l;

      i = main_map->l_searchlist.r_nlist;
      while (i-- > 0)
	{
	  /* Check each shared object to see if IBT and SHSTK are
	     enabled.  */
	  l = main_map->l_initfini[i];

	  /* Skip CET check for ld.so since ld.so is CET-enabled.  */
	  if (l == &GL(dl_rtld_map))
	    continue;

	  if (enable_ibt && !(l->l_cet & lc_ibt))
	    {
	      /* If IBT is enabled in executable and IBT isn't enabled in
		 this shard object, put all executable PT_LOAD segments
		 in legacy code page bitmap.  */

	      /* FIXME: Mark legacy region  */
	    }

	  /* SHSTK is enabled only if it is enabled in executable as
	     well as all shared objects.  */
	  enable_shstk = !!(l->l_cet & lc_shstk);

	  /* Stop if both IBT and SHSTCK are disabled.  */
	  if (!enable_ibt && !enable_shstk)
	    break;
	}
    }

  if (!enable_ibt || !enable_shstk)
    {
      /* FIXME: Disable IBT and/or SHSTK.  */
      ;
    }

  if (enable_ibt || enable_shstk)
    {
      /* FIXME: Lock CET if IBT or SHSTK is enabled.  */
      ;
    }

  _dl_init (main_map, argc, argv, env);
}
#endif
