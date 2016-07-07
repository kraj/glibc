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

#ifndef _SYS_SYSMACROS_H_OUTER

#ifndef __SYSMACROS_DEPRECATED_INCLUSION
# define _SYS_SYSMACROS_H_OUTER 1
#endif

/* If <sys/sysmacros.h> is included after <sys/types.h>, these macros
   will already be defined, and we need to redefine them without the
   deprecation warnings.  (If they are included in the opposite order,
   the outer #ifndef will suppress this entire file and the macros
   will be usable without warnings.)  */
#undef major
#undef minor
#undef makedev

/* This is the macro that must be defined to satisfy the misuse check
   in bits/sysmacros.h. */
#ifndef _SYS_SYSMACROS_H
# define _SYS_SYSMACROS_H 1

# include <features.h>
# include <bits/types.h>
# include <bits/sysmacros.h>

/* The extra "\n " moves gcc's [-Wdeprecated-declarations] annotation
   onto the next line.  */
# define __SYSMACROS_DEPRECATION_MSG(symbol)				     \
  "\n  In the GNU C Library, `" #symbol "' is defined by <sys/sysmacros.h>." \
  "\n  For historical compatibility, it is currently defined by"	     \
  "\n  <sys/types.h> as well, but we plan to remove this soon."		     \
  "\n  To use `" #symbol "', include <sys/sysmacros.h> directly."	     \
  "\n  If you did not intend to use a system-defined macro `" #symbol "',"   \
  "\n  you should #undef it after including <sys/types.h>."		     \
  "\n "

# define __SYSMACROS_DECL(rtype, name, proto)				     \
   extern rtype gnu_dev_##name proto __THROW __attribute_const__;	     \
   extern rtype __REDIRECT_NTH (__##name##_from_sys_types, proto,	     \
				gnu_dev_##name)				     \
    __attribute_const__							     \
    __attribute_deprecated_msg__ (__SYSMACROS_DEPRECATION_MSG (name));

# ifdef __USE_EXTERN_INLINES
#  define __SYSMACROS_IMPL(rtype, name, proto, body)			     \
    __SYSMACROS_DECL (rtype, name, proto)				     \
    __extension__ __extern_inline __attribute_const__ rtype		     \
    __NTH (gnu_dev_##name proto) { body }				     \
    __extension__ __extern_inline __attribute_const__ rtype		     \
    __NTH (__##name##_from_sys_types proto) { body }
# else
#  define __SYSMACROS_IMPL(rtype, name, proto, expr)			     \
    __SYSMACROS_DECL (rtype, name, proto)
# endif

__BEGIN_DECLS

__SYSMACROS_IMPL (unsigned int, major, (__dev_t __dev), __major_body (__dev))
__SYSMACROS_IMPL (unsigned int, minor, (__dev_t __dev), __minor_body (__dev))
__SYSMACROS_IMPL (__dev_t, makedev,
		  (unsigned int __major, unsigned int __minor),
		  __makedev_body (__major, __minor))

__END_DECLS

# undef __SYSMACROS_IMPL
# undef __SYSMACROS_DECL

# endif /* _SYS_SYSMACROS_H */

#ifdef __SYSMACROS_DEPRECATED_INCLUSION
# define major(dev) __major_from_sys_types (dev)
# define minor(dev) __minor_from_sys_types (dev)
# define makedev(maj, min) __makedev_from_sys_types (maj, min)
#else
# define major(dev) gnu_dev_major (dev)
# define minor(dev) gnu_dev_minor (dev)
# define makedev(maj, min) gnu_dev_makedev (maj, min)
#endif

#endif /* sys/sysmacros.h */
