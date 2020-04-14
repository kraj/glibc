/* Read a directory in reentrant mode.  Linux LFS version.
   Copyright (C) 1997-2020 Free Software Foundation, Inc.
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

/* When _DIRENT_MATCHES_DIRENT64 is defined we can alias 'readdir64' to
   'readdir'.  However the function signatures are not equal due
   different return types, so we need to suppress {__}readdir so weak
   and strong alias do not throw conflicting types errors.  */
#define readdir_r   __no_readdir_r_decl
#define __readdir_r __no___readdir_r_decl
#include <dirent.h>
#undef __readdir_r
#undef readdir_r

/* Read a directory entry from DIRP.  */
int
__readdir64_r (DIR *dirp, struct dirent64 *entry, struct dirent64 **result)
{
  struct dirent64 *dp;
  size_t reclen;

  __libc_lock_lock (dirp->lock);
  while (1)
    {
      dp = __readdir64_unlocked (dirp);
      if (dp == NULL)
	break;

      reclen = dp->d_reclen;
      if (reclen <= offsetof (struct dirent64, d_name) + NAME_MAX + 1)
	break;

      /* The record is very long.  It could still fit into the caller-supplied
	 buffer if we can skip padding at the end.  */
      size_t namelen = _D_EXACT_NAMLEN (dp);
      if (namelen <= NAME_MAX)
	{
	  reclen = offsetof (struct dirent64, d_name) + namelen + 1;
	  break;
	}

      /* The name is too long.  Ignore this file.  */
      dirp->errcode = ENAMETOOLONG;
      dp->d_ino = 0;
    }

  if (dp != NULL)
    {
      *result = memcpy (entry, dp, reclen);
      entry->d_reclen = reclen;
    }
  else
    *result = NULL;

  __libc_lock_unlock (dirp->lock);

  return dp != NULL ? 0 : dirp->errcode;
}


#if _DIRENT_MATCHES_DIRENT64
strong_alias (__readdir64_r, __readdir_r)
weak_alias (__readdir64_r, readdir_r)
weak_alias (__readdir64_r, readdir64_r)
#else
/* The compat code expects the 'struct direct' with d_ino being a __ino_t
   instead of __ino64_t.  */
# include <shlib-compat.h>
versioned_symbol (libc, __readdir64_r, readdir64_r, GLIBC_2_2);
# if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)
#  include <olddirent.h>

int
attribute_compat_text_section
__old_readdir64_r (DIR *dirp, struct __old_dirent64 *entry,
		   struct __old_dirent64 **result)
{
  struct __old_dirent64 *dp;
  size_t reclen;

  __libc_lock_lock (dirp->lock);

  while (1)
    {
      dp = __old_readdir64_unlocked (dirp);
      if (dp == NULL)
	break;

      reclen = dp->d_reclen;
      if (reclen <= offsetof (struct __old_dirent64, d_name) + NAME_MAX + 1)
	break;

      /* The record is very long.  It could still fit into the caller-supplied
	 buffer if we can skip padding at the end.  */
      size_t namelen = _D_EXACT_NAMLEN (dp);
      if (namelen <= NAME_MAX)
	{
	  reclen = offsetof (struct dirent64, d_name) + namelen + 1;
	  break;
	}

      /* The name is too long.  Ignore this file.  */
      dirp->errcode = ENAMETOOLONG;
      dp->d_ino = 0;
    }

  if (dp != NULL)
    {
      *result = memcpy (entry, dp, reclen);
      entry->d_reclen = reclen;
    }
  else
    *result = NULL;

  __libc_lock_unlock (dirp->lock);

  return dp != NULL ? 0 : dirp->errcode;
}

compat_symbol (libc, __old_readdir64_r, readdir64_r, GLIBC_2_1);
# endif /* SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)  */
#endif /* _DIRENT_MATCHES_DIRENT64  */
