/* Copyright (C) 2004-2020 Free Software Foundation, Inc.
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

#include <errno.h>
#include <mqueue.h>
#include <sysdep.h>

/* Remove message queue named NAME.  */
int
mq_unlink (const char *name)
{
  if (name[0] != '/')
    return __syscall_ret (-EINVAL);

  int ret = internal_syscall (__NR_mq_unlink, name + 1);

  /* While unlink can return either EPERM or EACCES, mq_unlink should
     return just EACCES.  */
  if (__syscall_err (ret))
    {
      if (ret == -EPERM)
	ret = -EACCES;
      return __syscall_ret (ret);
    }

  return ret;
}
