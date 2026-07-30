/* Data definitions for the multiple PT_GNU_RELRO test.
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

/* Assembly is used because compilers emit SHT_PROGBITS for
   __attribute__ ((section)) definitions and gas might warns when its
   name-based heuristic retypes .note.* sections; and  note sections
   must not have the SHF_WRITE flag for the same reason.  */

	.section .note.a, "a", %note
	.balign 8
	.globl relro_a
	.type relro_a, %object
relro_a:
	.dc.a 0xAAAA
	.size relro_a, .-relro_a

	.section .gap, "aw", %progbits
	.balign 8
	.globl gap_d
	.type gap_d, %object
gap_d:
	.dc.a 0xDDDD
	.size gap_d, .-gap_d

	.section .note.b, "a", %note
	.balign 8
	.globl relro_b
	.type relro_b, %object
relro_b:
	.dc.a 0xBBBB
	.size relro_b, .-relro_b
