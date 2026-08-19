/* Overly long symbol name shared by the relocation overflow test.
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

#ifndef _TST_RELOC_OVERFLOW_NAME_H
#define _TST_RELOC_OVERFLOW_NAME_H 1

#define CAT_(a, b) a##b
#define CAT(a, b) CAT_(a, b)

#define N16   xxxxxxxxxxxxxxxx
#define N32   CAT (N16, N16)
#define N64   CAT (N32, N32)
#define N128  CAT (N64, N64)
#define N256  CAT (N128, N128)
#define N512  CAT (N256, N256)
#define N1024 CAT (N512, N512)
#define N2048 CAT (N1024, N1024)
#define N4096 CAT (N2048, N2048)
#define N8192 CAT (N4096, N4096)

#define OVERFLOW_SYMBOL CAT (tst_reloc_overflow_symbol_, N8192)

#define STR_(x) #x
#define STR(x) STR_(x)

#endif
