/* Module with an out of range R_PPC*_ADDR16 relocation.
   Copyright (C) 2026 Free Software Foundation, Inc.
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

#include "tst-reloc-overflow-name.h"

/* _dl_reloc_overflow is only reached for relocation types that store an
   address into a field narrower than a pointer.  */

	.data
	.globl	refword
	.type	refword, @object
refword:
	.short	OVERFLOW_SYMBOL
	.size	refword, 2
