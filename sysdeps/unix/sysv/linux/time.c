/* Copyright (C) 2005-2017 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <time.h>

#include <sysdep.h>

#ifdef __NR_time

time_t
time (time_t *t)
{
  INTERNAL_SYSCALL_DECL (err);
  time_t res = INTERNAL_SYSCALL (time, err, 1, NULL);
  /* There cannot be any error.  */
  if (t != NULL)
    *t = res;
  return res;
}
libc_hidden_def (time)

/* 64-BIT TIME VERSION */

extern int __y2038_linux_support;

__time64_t
__time_t64 (__time64_t *t)
{
  INTERNAL_SYSCALL_DECL (err);
  __time64_t res;

  if (__y2038_linux_support)
  {
    /* TODO: implement using 64-bit time syscall */
  }

  res = INTERNAL_SYSCALL (time, err, 1, NULL);
  /* There cannot be any error.  */
  if (t != NULL)
    *t = res;
  return res;
}

#else

# include <sysdeps/posix/time.c>

#endif
