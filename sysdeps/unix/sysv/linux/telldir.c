/* Copyright (C) 1991-2020 Free Software Foundation, Inc.
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

#include <dirent.h>

#include <dirstream.h>
#include <telldir.h>

/* Return the current position of DIRP.  */
long int
telldir (DIR *dirp)
{
#ifndef __LP64__
  /* If the directory position fits in the packet structure returns it.
     Otherwise, check if the position is already been recorded in the
     dynamic array.  If not, add the new record.  */

  union dirstream_packed dsp;
  size_t i;

  __libc_lock_lock (dirp->lock);

  if (dirp->filepos < (1U << 31))
    {
      dsp.p.is_packed = 1;
      dsp.p.info = dirp->filepos;
      goto out;
    }

  dsp.l = -1;

  for (i = 0; i < dirstream_loc_size (&dirp->locs); i++)
    {
      struct dirstream_loc *loc = dirstream_loc_at (&dirp->locs, i);
      if (loc->filepos == dirp->filepos)
	break;
    }
  if (i == dirstream_loc_size (&dirp->locs))
    {
      dirstream_loc_add (&dirp->locs,
	(struct dirstream_loc) { dirp->filepos });
      if (dirstream_loc_has_failed (&dirp->locs))
	goto out;
    }

  dsp.p.is_packed = 0;
  /* This assignment might overflow, however most likely ENOMEM would happen
     long before.  */
  dsp.p.info = i;

out:
  __libc_lock_unlock (dirp->lock);

  return dsp.l;
#else
  long int ret;
  __libc_lock_lock (dirp->lock);
  ret = dirp->filepos;
  __libc_lock_unlock (dirp->lock);
  return ret;
#endif
}
