/* y2038 general kernel support indication.
   Copyright (C) 2018 Free Software Foundation, Inc.

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

/* By default glibc assumes the underlying kernel does not support Y2038 */ 
int __default_y2038_get_kernel_support (void)
{
  return 0;
}
weak_alias (__default_y2038_get_kernel_support, __y2038_get_kernel_support)

/* By default glibc just ignores Y2038 support indication setting */ 
int __default_y2038_set_kernel_support (int new with __attribute__ ((unused)))
{
  return 0;
}
weak_alias (__default_y2038_set_kernel_support, __y2038_set_kernel_support)
