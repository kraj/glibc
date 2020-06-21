/* Indirect system call.  Linux version.
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

#include <stdarg.h>
#include <sysdep.h>

long int
syscall (long int number, ...)
{
  va_list args;
  __syscall_arg_t arg0, arg1, arg2, arg3, arg4, arg5;

  /* Load varargs */
  va_start (args, number);
  arg0 = va_arg (args, __syscall_arg_t);
  arg1 = va_arg (args, __syscall_arg_t);
  arg2 = va_arg (args, __syscall_arg_t);
  arg3 = va_arg (args, __syscall_arg_t);
  arg4 = va_arg (args, __syscall_arg_t);
  arg5 = va_arg (args, __syscall_arg_t);
  va_end (args);

  return inline_syscall (number, arg0, arg1, arg2, arg3, arg4, arg5);
}
