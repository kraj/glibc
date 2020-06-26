/* Copyright (C) 2020 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#ifndef _ALPHA_SYSDEP_H
#define _ALPHA_SYSDEP_H 1

#ifdef __ASSEMBLER__
#include <asm/pal.h>
#include <alpha/regdef.h>

#define __LABEL(x)	x##:

#define LEAF(name, framesize)			\
  .globl name;					\
  .align 4;					\
  .ent name, 0;					\
  __LABEL(name)					\
  .frame sp, framesize, ra

#define ENTRY(name)				\
  .globl name;					\
  .align 4;					\
  .ent name, 0;					\
  __LABEL(name)					\
  .frame sp, 0, ra

/* Mark the end of function SYM.  */
#undef END
#define END(sym)	.end sym

#ifdef PROF
# define USEPV_PROF	std
#else
# define USEPV_PROF	no
#endif

#endif /* __ASSEMBLER__  */

#endif /* _LINUX_ALPHA_SYSDEP_H  */
