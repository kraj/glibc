/* y2038 Linux kernel support indication.
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

/* By default the underlying Linux kernel is assumed not to support Y2038.
 * Any Linux architecture may claim Y2038 kernel support by setting
 * __y2038_linux_support.
 */
int __y2038_linux_support = 0;

/* For Linux, Y2038 kernel support is determined by __y2038_linux_support  */

int __linux_y2038_get_kernel_support (void)
{
  return __y2038_linux_support;
}
strong_alias (__linux_y2038_get_kernel_support, __y2038_get_kernel_support)

int __linux_y2038_set_kernel_support (int new)
{
  int previous = __y2038_linux_support;
  __y2038_linux_support = new;
  return previous;
}
strong_alias (__linux_y2038_set_kernel_support, __y2038_set_kernel_support)
