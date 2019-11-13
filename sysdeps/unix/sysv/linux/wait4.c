/* Linux wait4 syscall implementation.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

#include <sys/wait.h>
#include <sys/resource.h>
#include <sysdep-cancel.h>

__pid_t
__wait4 (__pid_t pid, int *stat_loc, int options, struct rusage *usage)
{
#if __NR_wait4
   return SYSCALL_CANCEL (wait4, pid, stat_loc, options, usage);
#else
  idtype_t idtype = P_PID;

  if (pid < -1)
    {
      idtype = P_PGID;
      pid *= -1;
    }
  else if (pid == -1)
    {
      idtype = P_ALL;
    }
  else if (pid == 0)
    {
      /* Linux Kernels 5.4 support pid 0 with P_PGID to specify wait on
       * the current PID's group. Earlier kernels will return -EINVAL.
       */
      idtype = P_PGID;
    }

  options |= WEXITED;

  siginfo_t infop;
  if (SYSCALL_CANCEL (waitid, idtype, pid, &infop, options, usage) < 0)
    return -1;

  if (stat_loc)
    {
      *stat_loc = 0;
      switch (infop.si_code)
        {
        case CLD_EXITED:
          *stat_loc = W_EXITCODE (infop.si_status, 0);
          break;
        case CLD_DUMPED:
          *stat_loc = WCOREDUMP (infop.si_signo);
          /* Fallthrough */
        case CLD_KILLED:
          *stat_loc |= infop.si_status;
          break;
        case CLD_TRAPPED:
        case CLD_STOPPED:
          *stat_loc = W_STOPCODE (infop.si_status);
          break;
        case CLD_CONTINUED:
          *stat_loc = 0xffff;
          break;
	default:
	  *stat_loc = 0;
	  break;
        }
    }

  return infop.si_pid;
#endif
}
weak_alias (__wait4, wait4)
