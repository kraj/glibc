/* Definitions for memory copy functions, PA-RISC version.
   Copyright (C) 2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdeps/generic/memcopy.h>

/* Use a single double-word shift instead of two shifts and an ior.
   If the uses of MERGE were close to the computation of shl/shr,
   the compiler might have been able to create this itself.
   But instead that computation is well separated.

   Using an inline function instead of a macro is the easiest way
   to ensure that the types are correct.  */

#undef MERGE

extern void link_error(void);

static inline op_t
MERGE (op_t w0, int shl, op_t w1, int shr)
{
  op_t res;
  if (OPSIZ == 4)
    asm ("shrpw %1,%2,%%sar,%0" : "=r"(res) : "r"(w0), "r"(w1), "q"(shr));
  else if (OPSIZ == 8)
    asm ("shrpd %1,%2,%%sar,%0" : "=r"(res) : "r"(w0), "r"(w1), "q"(shr));
  else
    link_error (), res = 0;
  return res;
}
