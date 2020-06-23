/* vDSO common definition for Linux.
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
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

static inline long int
__internal_vsyscall0 (vdsop0_t vdsop, long int name)
{
  long int r;
  if (vdsop != NULL)
    {
      r = vdsop ();
      if (r == 0 || r != -ENOSYS)
	return r;
    }
  return internal_syscall (name);
}

static inline long int
__internal_vsyscall1 (vdsop1_t vdsop, long int name, __syscall_arg_t arg1)
{
  long int r;
  if (vdsop != NULL)
    {
      r = vdsop (arg1);
      if (r == 0 || r != -ENOSYS)
	return r;
    }
  return internal_syscall (name, arg1);
}

static inline long int
__internal_vsyscall2 (vdsop2_t vdsop, long int name, __syscall_arg_t arg1,
		      __syscall_arg_t arg2)
{
  long int r;
  if (vdsop != NULL)
    {
      r = vdsop (arg1, arg2);
      if (r == 0 || r != -ENOSYS)
	return r;
    }
  return internal_syscall (name, arg1, arg2);
}

static inline long int
__internal_vsyscall3 (vdsop3_t vdsop, long int name,  __syscall_arg_t arg1,
		      __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  long int r;
  if (vdsop != NULL)
    {
      r = vdsop (arg1, arg2, arg3);
      if (r == 0 || r != -ENOSYS)
	return r;
    }
  return internal_syscall (name, arg1, arg2, arg3);
}

static inline long int
__internal_vsyscall4 (vdsop4_t vdsop, long int name,  __syscall_arg_t arg1,
		      __syscall_arg_t arg2, __syscall_arg_t arg3,
		      __syscall_arg_t arg4)
{
  long int r;
  if (vdsop != NULL)
    {
      r = vdsop (arg1, arg2, arg3, arg4);
      if (r == 0 || r != -ENOSYS)
	return r;
    }
  return internal_syscall (name, arg1, arg2, arg3, arg4);
}

static inline long int
__internal_vsyscall5 (vdsop5_t vdsop, long int name,  __syscall_arg_t arg1,
		      __syscall_arg_t arg2, __syscall_arg_t arg3,
		      __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  long int r;
  if (vdsop != NULL)
    {
      r = vdsop (arg1, arg2, arg3, arg4, arg5);
      if (r == 0 || r != -ENOSYS)
	return r;
    }
  return internal_syscall (name, arg1, arg2, arg3, arg4, arg5);
}

static inline long int
__internal_vsyscall6 (vdsop6_t vdsop, long int name,  __syscall_arg_t arg1,
		      __syscall_arg_t arg2, __syscall_arg_t arg3,
		      __syscall_arg_t arg4, __syscall_arg_t arg5,
		      __syscall_arg_t arg6)
{
  long int r;
  if (vdsop != NULL)
    {
      r = vdsop (arg1, arg2, arg3, arg4, arg5, arg6);
      if (r == 0 || r != -ENOSYS)
	return r;
    }
  return internal_syscall (name, arg1, arg2, arg3, arg4, arg5, arg6);
}
