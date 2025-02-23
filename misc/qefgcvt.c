/* Compatibility functions for floating point formatting, long double version.
   Copyright (C) 1996-2025 Free Software Foundation, Inc.
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

#define ECVT qecvt
#define FCVT qfcvt
#define GCVT qgcvt
#define __ECVT __qecvt
#define __FCVT __qfcvt
#define __GCVT __qgcvt
#define __ECVT_R __qecvt_r
#define __FCVT_R __qfcvt_r
#define __EFGCVT_FREEMEM_PTR __libc_qefgcvt_freemem_ptr
#include <efgcvt-ldbl-macros.h>
#include <efgcvt-template.c>

#if LONG_DOUBLE_COMPAT (libc, GLIBC_2_0)
# define cvt_symbol(local, symbol) \
  versioned_symbol (libc, local, symbol, GLIBC_2_4)
#else
# define cvt_symbol(local, symbol) \
  strong_alias (local, symbol)
#endif
cvt_symbol (__qfcvt, qfcvt);
cvt_symbol (__qecvt, qecvt);
cvt_symbol (__qgcvt, qgcvt);
