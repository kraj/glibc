/* Define internal_function and private_function.  i386 version.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

/* On i386, we can make internal function calls (i.e., calls of functions
   within the same shared object) or private function calls (i.e., calls
   of functions between different shared objects of glibc) a bit faster
   by passing function parameters in registers.  */

#ifdef PROF
/* The mcount code relies on a normal frame pointer being on the stack
   to locate its caller.  */
# define internal_function	/* empty */
# define private_function	/* empty */
#else
# define internal_function	__attribute__ ((regparm (3), stdcall))
# define private_function	__attribute__ ((regparm (3), stdcall))
#endif
