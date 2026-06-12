/* Pointer obfuscation implenentation.  SH version.
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
#   define PTR_GUARD_SYM	__pointer_chk_guard
#  else
#   define PTR_GUARD_SYM	__pointer_chk_guard_local
#  endif
#  ifdef PIC
#   define PTR_GUARD_LOAD(tmp)						\
	mov	r0, r3;							\
	mova	.Lptrg_got, r0;						\
	mov.l	.Lptrg_got, tmp;					\
	add	r0, tmp;						\
	mov.l	.Lptrg_sym, r0;						\
	mov.l	@(r0, tmp), tmp;					\
	mov	r3, r0;							\
	mov.l	@tmp, tmp;						\
	bra	.Lptrg_end;						\
	 nop;								\
	.align	2;							\
.Lptrg_got:								\
	.long	_GLOBAL_OFFSET_TABLE_;					\
.Lptrg_sym:								\
	.long	PTR_GUARD_SYM@GOT;					\
.Lptrg_end:
#  else
#   define PTR_GUARD_LOAD(tmp)						\
	mov.l	.Lptrg_sym, tmp;					\
	mov.l	@tmp, tmp;						\
	bra	.Lptrg_end;						\
	 nop;								\
	.align	2;							\
.Lptrg_sym:								\
	.long	PTR_GUARD_SYM;						\
.Lptrg_end:
#  endif
#  define PTR_MANGLE(reg, tmp) \
     PTR_GUARD_LOAD (tmp); xor tmp,reg
#  define PTR_MANGLE2(reg, tmp) xor tmp,reg
#  define PTR_DEMANGLE(reg, tmp)        PTR_MANGLE (reg, tmp)
#  define PTR_DEMANGLE2(reg, tmp)       PTR_MANGLE2 (reg, tmp)
# else
#  include <stdint.h>
#  ifdef SHARED
extern uintptr_t __pointer_chk_guard attribute_relro;
#   define PTR_MANGLE(var) \
     (var) = (void *) ((uintptr_t) (var) ^ __pointer_chk_guard)
#  else
extern uintptr_t __pointer_chk_guard_local attribute_relro attribute_hidden;
#   define PTR_MANGLE(var) \
     (var) = (void *) ((uintptr_t) (var) ^ __pointer_chk_guard_local)
#  endif
#  define PTR_DEMANGLE(var)     PTR_MANGLE (var)
# endif
#endif

#endif /* POINTER_GUARD_H */
