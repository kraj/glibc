/* Copyright (C) 2006-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2006.

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

#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/poll.h>
#include <kernel-features.h>
#include <sysdep-cancel.h>

int __pselect64 (int nfds, fd_set *readfds, fd_set *writefds,
		 fd_set *exceptfds, const struct __timespec64 *timeout,
                 const sigset_t *sigmask)
{
  /* The Linux kernel can in some situations update the timeout value.
     We do not want that so use a local variable.  */
  struct __timespec64 tval;
  if (timeout != NULL)
    {
      tval = *timeout;
      timeout = &tval;
    }

  /* Note: the system call expects 7 values but on most architectures
     we can only pass in 6 directly.  If there is an architecture with
     support for more parameters a new version of this file needs to
     be created.  */
  struct
  {
    __syscall_ulong_t ss;
    __syscall_ulong_t ss_len;
  } data;

  data.ss = (__syscall_ulong_t) (uintptr_t) sigmask;
  data.ss_len = _NSIG / 8;

#ifdef __ASSUME_TIME64_SYSCALLS
# ifndef __NR_pselect6_time64
#  define __NR_pselect6_time64 __NR_pselect6
# endif
  return SYSCALL_CANCEL (pselect6_time64, nfds, readfds, writefds, exceptfds,
			 timeout, &data);
#else
# ifdef __NR_ppoll_time64
  int ret = SYSCALL_CANCEL (pselect6_time64, nfds, readfds, writefds,
			    exceptfds, timeout, &data);
  if (ret >= 0 || errno != ENOSYS)
    return ret;
# endif
  struct timespec ts32;
  if (timeout)
    {
      if (! in_time_t_range (timeout->tv_sec))
        {
          __set_errno (EOVERFLOW);
          return -1;
        }

      ts32 = valid_timespec64_to_timespec (*timeout);
    }

  return SYSCALL_CANCEL (pselect6, nfds, readfds, writefds, exceptfds,
			 timeout ? &ts32 : NULL, &data);
#endif /* __ASSUME_TIME64_SYSCALLS  */
}

#if __TIMESIZE != 64
int
__pselect (int nfds, fd_set *readfds, fd_set *writefds,
	     fd_set *exceptfds, const struct timespec *timeout,
             const sigset_t *sigmask)
{
  struct __timespec64 ts64;
  if (timeout)
    ts64 = valid_timespec_to_timespec64 (*timeout);

  return __pselect64 (nfds, readfds, writefds, exceptfds,
		      timeout ? &ts64 : NULL, sigmask);
}
#endif
#ifndef __pselect
weak_alias (__pselect, pselect)
#endif
