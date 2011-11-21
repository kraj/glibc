/* Copyright (C) 2011
   Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <alloca.h>
#include <errno.h>
#include <dirent.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sysdep.h>
#include <bp-checks.h>

/* For Linux we need a special version of this file since the
   definition of `struct dirent' is not the same for the kernel and
   the libc.  There is one additional field which might be introduced
   in the kernel structure in the future.

   Here is the kernel definition of `struct dirent64' as of 2.1.20:  */

struct kernel_dirent64
  {
    uint64_t		d_ino;
    int64_t		d_off;
    unsigned short int	d_reclen;
    unsigned char	d_type;
    char		d_name[256];
  };

#ifndef __GETDENTS
# define __GETDENTS __getdents
#endif
#ifndef DIRENT_TYPE
# define DIRENT_TYPE struct dirent
#endif
#ifndef DIRENT_SET_DP_INO
# define DIRENT_SET_DP_INO(dp, value) (dp)->d_ino = (value)
#endif

/* The problem here is that we cannot simply read the next NBYTES
   bytes.  We need to take the additional field into account.  We use
   some heuristic.  Assuming the directory contains names with 14
   characters on average we can compute an estimated number of entries
   which fit in the buffer.  Taking this number allows us to specify a
   reasonable number of bytes to read.  If we should be wrong, we can
   reset the file descriptor.  In practice the kernel is limiting the
   amount of data returned much more then the reduced buffer size.  */
ssize_t
internal_function
__GETDENTS (int fd, char *buf, size_t nbytes)
{
  ssize_t retval;
  off64_t last_offset = -1;
  union
    {
      struct kernel_dirent64 k;
      DIRENT_TYPE u;
      char b[1];
    } *kbuf = (void *) buf, *outp, *inp;
  size_t kbytes = nbytes;
  if (offsetof (DIRENT_TYPE, d_name)
      < offsetof (struct kernel_dirent64, d_name)
      && nbytes <= sizeof (DIRENT_TYPE))
    {
      kbytes = nbytes + offsetof (struct kernel_dirent64, d_name)
	- offsetof (DIRENT_TYPE, d_name);
      kbuf = __alloca(kbytes);
    }
  retval = INLINE_SYSCALL (getdents64, 3, fd, CHECK_N(kbuf, kbytes),
			   kbytes);
  const size_t size_diff = (offsetof (struct kernel_dirent64, d_name)
			    - offsetof (DIRENT_TYPE, d_name));

  /* Return the error if encountered.  */
  if (retval == -1)
    return -1;

  /* If the structure returned by the kernel is identical to what we
     need, don't do any conversions.  */
  if (offsetof (DIRENT_TYPE, d_name)
      == offsetof (struct kernel_dirent64, d_name)
      && sizeof (outp->u.d_ino) == sizeof (inp->k.d_ino)
      && sizeof (outp->u.d_off) == sizeof (inp->k.d_off))
    return retval;

  /* These two pointers might alias the same memory buffer.
     Standard C requires that we always use the same type for them,
     so we must use the union type.  */
  inp = kbuf;
  outp = (void *) buf;

  while (&inp->b < &kbuf->b + retval)
    {
      const size_t alignment = __alignof__ (DIRENT_TYPE);
      /* Since inp->k.d_reclen is already aligned for the kernel
	 structure this may compute a value that is bigger
	 than necessary.  */
      size_t old_reclen = inp->k.d_reclen;
      size_t new_reclen = ((old_reclen - size_diff + alignment - 1)
			   & ~(alignment - 1));

      /* Copy the data out of the old structure into temporary space.
	 Then copy the name, which may overlap if BUF == KBUF.  */
      const uint64_t d_ino = inp->k.d_ino;
      const int64_t d_off = inp->k.d_off;
      const uint8_t d_type = inp->k.d_type;

      memmove (outp->u.d_name, inp->k.d_name,
	       old_reclen - offsetof (struct kernel_dirent64, d_name));

      /* Now we have copied the data from INP and access only OUTP.  */

      DIRENT_SET_DP_INO (&outp->u, d_ino);
      outp->u.d_off = d_off;
      if ((sizeof (outp->u.d_ino) != sizeof (inp->k.d_ino)
	   && outp->u.d_ino != d_ino)
	  || (sizeof (outp->u.d_off) != sizeof (inp->k.d_off)
	      && outp->u.d_off != d_off))
	{
	  /* Overflow.  If there was at least one entry
	     before this one, return them without error,
	     otherwise signal overflow.  */
	  if (last_offset != -1)
	    {
	      __lseek64 (fd, last_offset, SEEK_SET);
	      return outp->b - buf;
	    }
	  __set_errno (EOVERFLOW);
	  return -1;
	}

      last_offset = d_off;
      outp->u.d_reclen = new_reclen;
      outp->u.d_type = d_type;

      inp = (void *) inp + old_reclen;
      outp = (void *) outp + new_reclen;
    }

  return outp->b - buf;
}

#undef __getdents64
weak_alias (__getdents, __getdents64);
