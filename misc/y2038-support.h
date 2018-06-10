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

/* Get Y2038 kernel support.
 * 0 means no suppport
 * > 0 means (some) support
 * < 0 means support is broken
 */
extern int __y2038_get_kernel_support (void);

/* Set Y2038 support.
 * 0 means no suppport
 * > 0 means (some) support
 * < 0 means support is broken
 * Architectures should call this with new > 0 as soon as they know that
 * their underlying kernel has Y2038 support.
 * Implementations should call this with new < 0 as soon as they detect
 * that a Y2038 kernel support failure occurred.
 * As a courtesy, the previous support indication is returned. */
extern int __y2038_set_kernel_support (int new);
