/* Unlock I/O stream.  Singlethreaded version.
   Copyright (C) 1996-2017 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <libio/libioP.h>

#undef funlockfile
#undef __funlockfile
#undef _IO_funlockfile

void
__funlockfile (FILE *stream)
{
  /* Do nothing.  Using this version does not do any locking.  */
}
strong_alias (__funlockfile, _IO_funlockfile)
weak_alias (__funlockfile, funlockfile)
libc_hidden_def (__funlockfile)
libc_hidden_def (_IO_funlockfile)

