/* Multiple versions of mempcpy. AARCH64 version.
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

#if IS_IN (libc)
# define mempcpy __redirect_mempcpy
# define __mempcpy __redirect___mempcpy
# define NO_MEMPCPY_STPCPY_REDIRECT
# define __NO_STRING_INLINES
# include <string.h>
# include <init-arch.h>

extern __typeof (__redirect_mempcpy) __mempcpy_generic attribute_hidden;
extern __typeof (__redirect_mempcpy) __mempcpy_thunderx attribute_hidden;
extern __typeof (__redirect_mempcpy) __mempcpy_thunderx2 attribute_hidden;
extern __typeof (__redirect_mempcpy) __mempcpy_falkor attribute_hidden;

# undef mempcpy
# undef __mempcpy

libc_ifunc_redirected (__redirect___mempcpy, __mempcpy,
		       (IS_THUNDERX (midr)
		       ? __mempcpy_thunderx
		       : (IS_FALKOR (midr) || IS_PHECDA (midr)
			 ? __mempcpy_falkor
			 : (IS_THUNDERX2 (midr) || IS_THUNDERX2PA (midr)
			   ? __mempcpy_thunderx2
			   : __mempcpy_generic))));
weak_alias (__mempcpy, mempcpy)
#endif
