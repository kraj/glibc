/* Multiple versions of __rintf.
   Copyright (C) 2017-2025 Free Software Foundation, Inc.
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

#include <sysdeps/x86/isa-level.h>
#if MINIMUM_X86_ISA_LEVEL < SSE4_1_X86_ISA_LEVEL
# define NO_MATH_REDIRECT
# include <libm-alias-float.h>

# define rintf __redirect_rintf
# define __rintf __redirect___rintf
# include <math.h>
# undef rintf
# undef __rintf

# define SYMBOL_NAME rintf
# include "ifunc-sse4_1.h"

libc_ifunc_redirected (__redirect_rintf, __rintf, IFUNC_SELECTOR ());
libm_alias_float (__rint, rint)
#endif
