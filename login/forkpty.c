/* Copyright (C) 1998-2025 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <utmp.h>
#include <pty.h>
#include <shlib-compat.h>

int
__forkpty (int *pptmx, char *name, const struct termios *termp,
	   const struct winsize *winp)
{
  int ptmx, terminal, pid;

  if (openpty (&ptmx, &terminal, name, termp, winp) == -1)
    return -1;

  switch (pid = __fork ())
    {
    case -1:
      __close (ptmx);
      __close (terminal);
      return -1;
    case 0:
      /* Child.  */
      __close (ptmx);
      if (login_tty (terminal))
	_exit (1);

      return 0;
    default:
      /* Parent.  */
      *pptmx = ptmx;
      __close (terminal);

      return pid;
    }
}
versioned_symbol (libc, __forkpty, forkpty, GLIBC_2_34);
libc_hidden_ver (__forkpty, forkpty)

#if OTHER_SHLIB_COMPAT (libutil, GLIBC_2_0, GLIBC_2_34)
compat_symbol (libutil, __forkpty, forkpty, GLIBC_2_0);
#endif
