/* Pointer obfuscation implenentation.  s390x version.
   Copyright (C) 2005-2026 Free Software Foundation, Inc.
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

#if IS_IN (rtld)
# include <sysdeps/generic/pointer_guard.h>
#else
# ifdef __ASSEMBLER__
#  ifdef SHARED
#   define PTR_GUARD_LOAD(tmpreg)					\
	larl	tmpreg,__pointer_chk_guard@GOTENT;			\
	lg	tmpreg,0(tmpreg);					\
	lg	tmpreg,0(tmpreg)
#  else
#   define PTR_GUARD_LOAD(tmpreg)					\
	larl	tmpreg,__pointer_chk_guard_local;			\
	lg	tmpreg,0(tmpreg)
#  endif
#  define PTR_MANGLE(reg, tmpreg) \
	PTR_GUARD_LOAD (tmpreg);					\
	xgr	reg,tmpreg
#  define PTR_MANGLE2(reg, tmpreg) \
	xgr	reg,tmpreg
#  define PTR_DEMANGLE(reg, tmpreg) PTR_MANGLE (reg, tmpreg)
#  define PTR_DEMANGLE2(reg, tmpreg) PTR_MANGLE2 (reg, tmpreg)
# else
#  include <stdint.h>
#  ifdef SHARED
extern uintptr_t __pointer_chk_guard attribute_relro;
#   define PTR_GUARD_VALUE	__pointer_chk_guard
#  else
extern uintptr_t __pointer_chk_guard_local attribute_relro attribute_hidden;
#   define PTR_GUARD_VALUE	__pointer_chk_guard_local
#  endif
#  define PTR_MANGLE(var) \
  (var) = (void *) ((uintptr_t) (var) ^ PTR_GUARD_VALUE)
#  define PTR_DEMANGLE(var)	PTR_MANGLE (var)
# endif
#endif

#endif /* POINTER_GUARD_H */
