/* Linux readdir internal implementation details.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#ifndef _DIRSTREAM_NOLFS_H
#define _DIRSTREAM_NOLFS_H

#if !_DIRENT_MATCHES_DIRENT64
# include <dirstream.h>
# include <olddirent.h>

/* getdents64 is used internally for both LFS and non-LFS implementations.
   The non-LFS interface reserves part of the allocated buffer to return the
   non-LFS 'struct dirent' entry.  */

/* This defines the reserved space size on DIR internal buffer to use as the
   returned 'struct dirent' from a 'readdir' call.

   The largest possible practical length of the d_name member are 255
   Unicode characters in UTF-8 encoding, so d_name is 766 bytes long, plus
   10 bytes from header, for a total of 776 bytes total.

   Also it should take in cosideration the alignment requirement for
   getdents64 call.  */
enum { return_buffer_size = 1024
			    + sizeof (off64_t)
			    - _Alignof (((struct __dirstream) {0}).data) };

_Static_assert ((_Alignof (((struct __dirstream) {0}).data)
		 + return_buffer_size) % sizeof (off64_t) == 0,
		"return_buffer_size does not align the buffer properly");

/* Return the avaliable buffer size to use with getdents64 calls.  */
static inline size_t
dirstream_alloc_size (struct __dirstream *ds)
{
  return ds->allocation - return_buffer_size;
}

/* Return the start of the allocated buffer minus the reserved part to use on
   non-LFS readdir call.  */
static inline void *
dirstream_data (struct __dirstream *ds)
{
  return (char *) ds->data + return_buffer_size;
}

/* Return the allocated buffer used on non-LFS readdir call.  */
static inline struct dirent *
dirstream_ret (struct __dirstream *ds)
{
  return (struct dirent *) ds->data;
}

/* Return the current dirent64 entry from the reserved buffer used on
   getdent64.  */
static inline struct dirent64 *
dirstream_entry (struct __dirstream *ds)
{
  size_t offset = return_buffer_size + ds->offset;
  return (struct dirent64 *) ((char *) ds->data + offset);
}

/* Copy one obtained entry from 'getdents64' call to the reserved space
   on DS allocated buffer and updated its internal state.  */
static inline struct dirent *
dirstream_ret_entry (struct __dirstream *ds)
{
  struct dirent64 *dp64 = dirstream_entry (ds);
  struct dirent *dp = dirstream_ret (ds);

  dp->d_ino = dp64->d_ino;

  dp->d_off = dp64->d_off;

  const size_t size_diff = (offsetof (struct dirent64, d_name)
			    - offsetof (struct dirent, d_name));
  const size_t alignment = _Alignof (struct dirent);
  size_t new_reclen = (dp64->d_reclen - size_diff + alignment - 1)
		       & ~(alignment - 1);
  if (new_reclen > return_buffer_size)
    /* Overflow.  */
    return NULL;
  dp->d_reclen = new_reclen;

  dp->d_type = dp64->d_type;

  memcpy (dp->d_name, dp64->d_name,
	  dp64->d_reclen - offsetof (struct dirent64, d_name));

  ds->offset += dp64->d_reclen;
  ds->filepos = dp64->d_off;

  return dp;
}

/* Return the allocated buffer used on LFS compat readdir call.  */
static inline struct __old_dirent64 *
dirstream_ret64_compat (struct __dirstream *ds)
{
  return (struct __old_dirent64 *) ds->data;
}

static inline struct __old_dirent64 *
dirstream_ret_entry64_compat (struct __dirstream *ds)
{
  struct dirent64 *dp64 = dirstream_entry (ds);
  struct __old_dirent64 *dp64_compat = dirstream_ret64_compat (ds);

  dp64_compat->d_ino = dp64->d_ino;
  if (dp64_compat->d_ino != dp64->d_ino)
    /* Overflow.  */
    return NULL;

  dp64_compat->d_off = dp64->d_off;

  const size_t size_diff = (offsetof (struct dirent64, d_name)
			    - offsetof (struct __old_dirent64, d_name));
  const size_t alignment = _Alignof (struct __old_dirent64);
  size_t new_reclen  = (dp64->d_reclen - size_diff + alignment - 1)
			& ~(alignment - 1);
  if (new_reclen > return_buffer_size)
    /* Overflow.  */
    return NULL;

  /* The compat symbol report the kernel obtained d_reclen, even though
     it has an incompatible dirent layout.  */
  dp64_compat->d_reclen = dp64->d_reclen;

  dp64_compat->d_type = dp64->d_type;

  memcpy (dp64_compat->d_name, dp64->d_name,
	  dp64->d_reclen - offsetof (struct dirent64, d_name));

  ds->offset += dp64->d_reclen;
  ds->filepos = dp64->d_off;

  return dp64_compat;
}

#else
/* No need to reserve an buffer space if dirent has already LFS support.  */
enum { return_buffer_size = 0 };
#endif /* _DIRENT_MATCHES_DIRENT64  */
#endif
