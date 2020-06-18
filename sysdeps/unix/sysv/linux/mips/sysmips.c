/* sysmips system call for Linux/MIPS.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <sys/sysmips.h>
#include <sysdep.h>
#include <stdarg.h>

int
sysmips (const int cmd, ...)
{
  va_list va;
  long int arg1 = 0;
  long int arg2 = 0;

  switch (cmd)
    {
    case MIPS_ATOMIC_SET:
      va_start (va, cmd);
      arg1 = va_arg (va, long int);
      arg2 = va_arg (va, long int);
      va_end (va);
      break;
    case MIPS_FIXADE:
      va_start (va, cmd);
      arg1 = va_arg (va, long int);
      va_end (va);
      break;
    default:
      break;
    }
  return inline_syscall (__NR_sysmips, cmd, arg1, arg2);
}
