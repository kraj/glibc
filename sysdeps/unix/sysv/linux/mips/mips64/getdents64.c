/* Get directory entries.  Linux/MIPSn64 LFS version.
   Copyright (C) 2018-2019 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>
#include <sys/param.h>
#include <unistd.h>
#include <limits.h>

ssize_t
__getdents64 (int fd, void *buf, size_t nbytes)
{
  /* The system call takes an unsigned int argument, and some length
     checks in the kernel use an int type.  */
  if (nbytes > INT_MAX)
    nbytes = INT_MAX;

#ifdef __NR_getdents64
  static bool getdents64_supportted = true;
  if (atomic_load_relaxed (&getdents64_supportted))
    {
      ssize_t ret = INLINE_SYSCALL_CALL (getdents64, fd, buf, nbytes);
      if (ret >= 0 || errno != ENOSYS)
	return ret;

      atomic_store_relaxed (&getdents64_supportted, false);
    }
#endif

  /* Unfortunately getdents64 was only wire-up for MIPS n64 on Linux 3.10.
     If the syscall is not available it need to fallback to the non-LFS one.
     Also to avoid an unbounded allocation through VLA/alloca or malloc (which
     would make the syscall non async-signal-safe) it uses a limited buffer.
     This is sub-optimal for large NBYTES, however this is a fallback
     mechanism to emulate a syscall that kernel should provide.   */

  enum { KBUF_SIZE = 1024 };
  struct kernel_dirent
  {
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short int d_reclen;
    char d_name[1];
  } kbuf[KBUF_SIZE / sizeof (struct kernel_dirent)];
  size_t kbuf_size = nbytes < KBUF_SIZE ? nbytes : KBUF_SIZE;

  struct dirent64 *dp = (struct dirent64 *) buf;

  size_t nb = 0;
  off64_t last_offset = -1;

  ssize_t r;
  while ((r = INLINE_SYSCALL_CALL (getdents, fd, kbuf, kbuf_size)) > 0)
    {
      struct kernel_dirent *skdp, *kdp;
      skdp = kdp = kbuf;

      while ((char *) kdp < (char *) skdp + r)
	{
	  const size_t alignment = _Alignof (struct dirent64);
	  size_t new_reclen = ((kdp->d_reclen + alignment - 1)
			      & ~(alignment - 1));
	  if (nb + new_reclen > nbytes)
	    {
		/* The new entry will overflow the input buffer, rewind to
		   last obtained entry and return.  */
	       __lseek64 (fd, last_offset, SEEK_SET);
	       goto out;
	    }
	  nb += new_reclen;

	  dp->d_ino = kdp->d_ino;
	  dp->d_off = last_offset = kdp->d_off;
	  dp->d_reclen = new_reclen;
	  dp->d_type = *((char *) kdp + kdp->d_reclen - 1);
	  memcpy (dp->d_name, kdp->d_name,
		  kdp->d_reclen - offsetof (struct kernel_dirent, d_name));

	  dp = (struct dirent64 *) ((char *) dp + new_reclen);
	  kdp = (struct kernel_dirent *) (((char *) kdp) + kdp->d_reclen);
	}
    }

out:
  return (char *) dp - (char *) buf;
}
libc_hidden_def (__getdents64)
weak_alias (__getdents64, getdents64)

#if _DIRENT_MATCHES_DIRENT64
strong_alias (__getdents64, __getdents)
#endif
