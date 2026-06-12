/* Pointer obfuscation implenentation.  i386 version.
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
/* We cannot use the thread descriptor because in ld.so we use setjmp
   earlier than the descriptor is initialized.  */
# include <sysdeps/generic/pointer_guard.h>
#else
# ifdef __ASSEMBLER__
#  include <sysdep.h>
/* The PIC variants clobber TMP, a register name suffix as expected by
   LOAD_PIC_REG (e.g. dx for %edx), to load the guard value.  */
#  ifdef SHARED
/* The global guard is reached through the GOT.  */
#   define PTR_MANGLE(reg, tmp)	LOAD_PIC_REG (tmp);			      \
				movl __pointer_chk_guard@GOT(%e##tmp), %e##tmp; \
				xorl (%e##tmp), reg;			      \
				roll $9, reg
#   define PTR_DEMANGLE(reg, tmp) \
				rorl $9, reg;				      \
				LOAD_PIC_REG (tmp);			      \
				movl __pointer_chk_guard@GOT(%e##tmp), %e##tmp; \
				xorl (%e##tmp), reg
#  elif defined PIC
/* Static PIE: the module-local guard is reached via @GOTOFF.  */
#   define PTR_MANGLE(reg, tmp)	LOAD_PIC_REG (tmp);			      \
				xorl __pointer_chk_guard_local@GOTOFF(%e##tmp), reg; \
				roll $9, reg
#   define PTR_DEMANGLE(reg, tmp) \
				rorl $9, reg;				      \
				LOAD_PIC_REG (tmp);			      \
				xorl __pointer_chk_guard_local@GOTOFF(%e##tmp), reg
#  else
/* Position-dependent code addresses the guard directly.  */
#   define PTR_MANGLE(reg, tmp)	xorl __pointer_chk_guard_local, reg;	      \
				roll $9, reg
#   define PTR_DEMANGLE(reg, tmp) \
				rorl $9, reg;				      \
				xorl __pointer_chk_guard_local, reg
#  endif
# else
#  include <stdbit.h>
#  include <stdint.h>
#  ifdef SHARED
extern uintptr_t __pointer_chk_guard attribute_relro;
#   define PTR_GUARD_VALUE	__pointer_chk_guard
#  else
extern uintptr_t __pointer_chk_guard_local attribute_relro attribute_hidden;
#   define PTR_GUARD_VALUE	__pointer_chk_guard_local
#  endif
#  define PTR_MANGLE(var)						      \
    do {								      \
      (var) = (__typeof (var)) ((uintptr_t) (var) ^ PTR_GUARD_VALUE);	      \
      (var) = (__typeof (var)) stdc_rotate_left ((uintptr_t) (var),	      \
						 2 * sizeof (uintptr_t) + 1); \
    } while (0)
#  define PTR_DEMANGLE(var)						      \
    do {								      \
      (var) = (__typeof (var)) stdc_rotate_right ((uintptr_t) (var),	      \
						  2 * sizeof (uintptr_t) + 1); \
      (var) = (__typeof (var)) ((uintptr_t) (var) ^ PTR_GUARD_VALUE);	      \
    } while (0)
# endif
#endif

#endif /* POINTER_GUARD_H */
