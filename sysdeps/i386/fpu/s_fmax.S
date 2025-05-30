/* Compute maximum of two numbers, regarding NaN as missing argument.
   Copyright (C) 1997-2025 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <libm-alias-double.h>

	.text
ENTRY(__fmax)
	fldl	12(%esp)	// y
	fxam
	fnstsw
	fldl	4(%esp)		// y : x

	andb	$0x45, %ah
	cmpb	$0x01, %ah
	je	1f		// y == NaN

	fucom	%st(1)
	fnstsw
	sahf
	jnc	1f

	fxch	%st(1)
1:	fstp	%st(1)

	ret
END(__fmax)
libm_alias_double (__fmax, fmax)
