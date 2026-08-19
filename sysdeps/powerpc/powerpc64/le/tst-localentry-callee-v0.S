/* Callee with a local entry point of zero, used at link time.
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

#include "tst-localentry-name.h"

/* The modules required st_other to have localentry:0, because that is what
   makes ld optimize the module's call under --plt-localentry.

   This is written in assembly to assure the code generation decision
   regardless of compiler flags (like stack protection) which change how
   the leaf function returns the value.  */

	.abiversion 2
	.section	".text"
	.globl	LOCALENTRY_SYMBOL
	.type	LOCALENTRY_SYMBOL, @function
LOCALENTRY_SYMBOL:
	.localentry	LOCALENTRY_SYMBOL, 0
	li	3, 42
	blr
	.size	LOCALENTRY_SYMBOL, .-LOCALENTRY_SYMBOL
