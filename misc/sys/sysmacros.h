/* Definitions of macros to access `dev_t' values.
   Copyright (C) 1996-2015 Free Software Foundation, Inc.
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

#ifndef _SYS_SYSMACROS_H
#define _SYS_SYSMACROS_H 1

#include <features.h>
#include <bits/types.h>
#include <bits/sysmacros.h>

#define __SYSMACROS_DECL(rtype, name, proto)                            \
  extern rtype gnu_dev_##name proto __THROW __attribute_const__;

#ifdef __USE_EXTERN_INLINES
# define __SYSMACROS_IMPL(rtype, name, proto, body)             \
  __SYSMACROS_DECL (rtype, name, proto)                         \
  __extension__ __extern_inline __attribute_const__ rtype       \
  __NTH (gnu_dev_##name proto) { body }
#else
# define __SYSMACROS_IMPL(rtype, name, proto, expr)    \
  __SYSMACROS_DECL (rtype, name, proto)
#endif

__BEGIN_DECLS

__SYSMACROS_IMPL (unsigned int, major, (__dev_t __dev), __major_body (__dev))
__SYSMACROS_IMPL (unsigned int, minor, (__dev_t __dev), __minor_body (__dev))
__SYSMACROS_IMPL (__dev_t, makedev,
		  (unsigned int __major, unsigned int __minor),
		  __makedev_body (__major, __minor))

__END_DECLS

#undef __SYSMACROS_IMPL
#undef __SYSMACROS_DECL

#define major(dev) gnu_dev_major (dev)
#define minor(dev) gnu_dev_minor (dev)
#define makedev(maj, min) gnu_dev_makedev (maj, min)

#endif /* sys/sysmacros.h */
