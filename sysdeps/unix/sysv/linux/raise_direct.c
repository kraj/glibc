/* Internal function to send a signal to itself.  Linux version.
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

#include <sysdep.h>
#include <pthread.h>
#include <unistd.h>

int
__raise_direct (int signo)
{
  /* Make direct syscalls and avoid reuse cached values.  */
  pid_t pid = INTERNAL_SYSCALL_CALL (getpid);
  pid_t tid = INTERNAL_SYSCALL_CALL (gettid);
  int ret = INTERNAL_SYSCALL_CALL (tgkill, pid, tid, signo);
  return INTERNAL_SYSCALL_ERROR_P (ret) ? INTERNAL_SYSCALL_ERRNO (ret) : 0;
}
