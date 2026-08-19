/* Callee with a non-zero local entry point, substituted at run time.
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

/* The same symbol with a global entry sequence that sets up r2, so st_other
   says localentry:8.  Resolving the module's optimized call to this
   definition is the ABI violation ld.so has to reject.  */

	.abiversion 2
	.section	".text"
	.globl	LOCALENTRY_SYMBOL
	.type	LOCALENTRY_SYMBOL, @function
LOCALENTRY_SYMBOL:
0:	addis	2, 12, .TOC.-0b@ha
	addi	2, 2, .TOC.-0b@l
	.localentry	LOCALENTRY_SYMBOL, .-LOCALENTRY_SYMBOL
	li	3, 43
	blr
	.size	LOCALENTRY_SYMBOL, .-LOCALENTRY_SYMBOL
