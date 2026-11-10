/* Linux poll implementation.
   Copyright (C) 2017-2026 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/poll.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>

int
__poll (struct pollfd *fds, nfds_t nfds, int timeout)
{
#ifdef __NR_poll
  return SYSCALL_CANCEL (poll, fds, nfds, timeout);
#else
  /* The timeout always fits in the time32 ppoll.  */
# ifdef __ASSUME_TIME64_SYSCALLS
#  ifndef __NR_ppoll_time64
#   define __NR_ppoll_time64 __NR_ppoll
#  endif
  struct __timespec64 timeout_ts;
  struct __timespec64 *timeout_ts_p = NULL;
# else
  struct timespec timeout_ts;
  struct timespec *timeout_ts_p = NULL;
# endif

  if (timeout >= 0)
    {
      timeout_ts.tv_sec = timeout / 1000;
      timeout_ts.tv_nsec = (timeout % 1000) * 1000000;
      timeout_ts_p = &timeout_ts;
    }

# ifdef __ASSUME_TIME64_SYSCALLS
  return SYSCALL_CANCEL (ppoll_time64, fds, nfds, timeout_ts_p, NULL, 0);
# else
  return SYSCALL_CANCEL (ppoll, fds, nfds, timeout_ts_p, NULL, 0);
# endif
#endif
}
libc_hidden_def (__poll)
weak_alias (__poll, poll)
strong_alias (__poll, __libc_poll)
