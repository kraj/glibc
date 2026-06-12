/* Pointer obfuscation implenentation.  64-bit SPARC version.
   Copyright (C) 2006-2026 Free Software Foundation, Inc.
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

#ifndef POINTER_GUARD_H
#define POINTER_GUARD_H

#ifdef __ASSEMBLER__
# include <sysdep.h>
# if IS_IN (rtld) || !defined SHARED
#  define PTR_GUARD_SYM	__pointer_chk_guard_local
# else
#  define PTR_GUARD_SYM	__pointer_chk_guard
# endif
# ifdef PIC
/* Load the guard through the GOT.  SETUP_PIC_REG_LEAF computes the GOT
   pointer in %o2 while preserving %o7, using %o3 as a scratch register.  */
#  define PTR_GUARD_LOAD(tmpreg)					\
	SETUP_PIC_REG_LEAF(o2, o3);					\
	sethi	%gdop_hix22(PTR_GUARD_SYM), tmpreg;			\
	xor	tmpreg, %gdop_lox10(PTR_GUARD_SYM), tmpreg;		\
	ldx	[%o2 + tmpreg], tmpreg, %gdop(PTR_GUARD_SYM);		\
	ldx	[tmpreg], tmpreg
# else
#  define PTR_GUARD_LOAD(tmpreg)					\
	sethi	%hi(PTR_GUARD_SYM), tmpreg;				\
	ldx	[tmpreg + %lo(PTR_GUARD_SYM)], tmpreg
# endif
# define PTR_MANGLE(dreg, reg, tmpreg) \
	PTR_GUARD_LOAD (tmpreg);					\
	xor	reg, tmpreg, dreg
# define PTR_DEMANGLE(dreg, reg, tmpreg) PTR_MANGLE (dreg, reg, tmpreg)
# define PTR_MANGLE2(dreg, reg, tmpreg) \
	xor	reg, tmpreg, dreg
# define PTR_DEMANGLE2(dreg, reg, tmpreg) PTR_MANGLE2 (dreg, reg, tmpreg)
#else
# include <stdint.h>
# if IS_IN (rtld) || !defined SHARED
extern uintptr_t __pointer_chk_guard_local attribute_relro attribute_hidden;
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ __pointer_chk_guard_local)
# else
extern uintptr_t __pointer_chk_guard attribute_relro;
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ __pointer_chk_guard)
# endif
# define PTR_DEMANGLE(var)     PTR_MANGLE (var)
#endif

#endif /* POINTER_GUARD_H */
