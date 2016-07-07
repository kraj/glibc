/* Definitions of macros to access `dev_t' values.
   Copyright (C) 1996-2016 Free Software Foundation, Inc.
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

#ifndef _BITS_SYSMACROS_H
#define _BITS_SYSMACROS_H 1

#ifndef _SYS_SYSMACROS_H
# error "Never include <bits/sysmacros.h> directly; use <sys/sysmacros.h> instead."
#endif

/* dev_t in glibc is a 64-bit quantity, with 32-bit major and minor numbers.
   Our default encoding is MMMM Mmmm mmmM MMmm, where M is a hex digit of
   the major number and m is a hex digit of the minor number.  This is
   downward compatible with legacy systems where dev_t is 16 bits wide,
   encoded as MMmm.  It is also downward compatible with the Linux kernel,
   which (as of 2016) uses 32-bit dev_t, encoded as mmmM MMmm.

   Systems that use an incompatible encoding for dev_t should override this
   file in the appropriate sysdeps subdirectory.  The macros __major_body,
   __minor_body, and __makedev_body are used as the bodies of inline
   functions, and their arguments are guaranteed to be the names of
   parameter variables, so it is safe to use them multiple times and
   unnecessary to parenthesize them.  See sys/sysmacros.h for details.  */

#define __major_body(__dev_)                                    \
  unsigned int __major_;                                        \
  __major_  = ((__dev_ & (__dev_t) 0x00000000000fff00u) >>  8); \
  __major_ |= ((__dev_ & (__dev_t) 0xfffff00000000000u) >> 32); \
  return __major_;

#define __minor_body(__dev_)                                    \
  unsigned int __minor_;                                        \
  __minor_  = ((__dev_ & (__dev_t) 0x00000000000000ffu) >>  0); \
  __minor_ |= ((__dev_ & (__dev_t) 0x00000ffffff00000u) >> 12); \
  return __minor_;

#define __makedev_body(__major_, __minor_)                      \
  __dev_t __dev_;                                               \
  __dev_  = (((__dev_t) (__minor_ & 0x000000ffu)) <<  0);       \
  __dev_ |= (((__dev_t) (__minor_ & 0xffffff00u)) << 12);       \
  __dev_ |= (((__dev_t) (__major_ & 0x00000fffu)) <<  8);       \
  __dev_ |= (((__dev_t) (__major_ & 0xfffff000u)) << 32);       \
  return __dev_;

#endif /* bits/sysmacros.h */
