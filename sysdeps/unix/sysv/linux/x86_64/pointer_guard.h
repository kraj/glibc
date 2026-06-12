/* Pointer obfuscation implenentation.  x86-64 version.
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

#include <x86-lp_size.h>
#include <tcb-offsets.h>

#if (IS_IN (rtld) \
     || (!defined SHARED && (IS_IN (libc) || IS_IN (libpthread))))
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(reg)	xor __pointer_chk_guard_local(%rip), reg;     \
				rol $2*LP_SIZE+1, reg
#  define PTR_DEMANGLE(reg)	ror $2*LP_SIZE+1, reg;			      \
				xor __pointer_chk_guard_local(%rip), reg
# else
#  include <stdbit.h>
#  include <stdint.h>
extern uintptr_t __pointer_chk_guard_local attribute_relro attribute_hidden;
#  define PTR_MANGLE(var)						      \
    do {								      \
      (var) = (__typeof (var)) ((uintptr_t) (var)			      \
				^ __pointer_chk_guard_local);		      \
      (var) = (__typeof (var)) stdc_rotate_left ((uintptr_t) (var),	      \
						 2 * sizeof (uintptr_t) + 1); \
    } while (0)
#  define PTR_DEMANGLE(var)						      \
    do {								      \
      (var) = (__typeof (var)) stdc_rotate_right ((uintptr_t) (var),	      \
						  2 * sizeof (uintptr_t) + 1); \
      (var) = (__typeof (var)) ((uintptr_t) (var)			      \
				^ __pointer_chk_guard_local);		      \
    } while (0)
# endif
#else
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(reg)	mov __pointer_chk_guard@GOTPCREL(%rip), %R11_LP;\
				xor (%R11_LP), reg;			      \
				rol $2*LP_SIZE+1, reg
#  define PTR_DEMANGLE(reg)	ror $2*LP_SIZE+1, reg;			      \
				mov __pointer_chk_guard@GOTPCREL(%rip), %R11_LP;\
				xor (%R11_LP), reg
# else
#  include <stdbit.h>
#  include <stdint.h>
extern uintptr_t __pointer_chk_guard attribute_relro;
#  define PTR_MANGLE(var)						      \
    do {								      \
      (var) = (__typeof (var)) ((uintptr_t) (var)			      \
				^ __pointer_chk_guard);			      \
      (var) = (__typeof (var)) stdc_rotate_left ((uintptr_t) (var),	      \
						 2 * sizeof (uintptr_t) + 1); \
    } while (0)
#  define PTR_DEMANGLE(var)						      \
    do {								      \
      (var) = (__typeof (var)) stdc_rotate_right ((uintptr_t) (var),	      \
						  2 * sizeof (uintptr_t) + 1); \
      (var) = (__typeof (var)) ((uintptr_t) (var)			      \
				^ __pointer_chk_guard);			      \
    } while (0)
# endif
#endif

#endif /* POINTER_GUARD_H */
