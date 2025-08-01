/* Test whether a file descriptor refers to a terminal.  Linux version.
   Copyright (C) 1991-2025 Free Software Foundation, Inc.
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

#include <termios_internals.h>

/* Return 1 if FD is a terminal, 0 if not. This simply does a
   TCGETS2 ioctl into a dummy buffer without parsing the result. */
int
__isatty (int fd)
{
  struct termios2 k_termios;
  return INLINE_SYSCALL_CALL (ioctl, fd, TCGETS2, &k_termios) == 0;
}
weak_alias (__isatty, isatty)
