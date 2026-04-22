/* Define MALLOC_ALIGNMENT for malloc.  i386 version.
   Copyright (C) 2017-2026 Free Software Foundation, Inc.
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

#ifndef _I386_MALLOC_ALIGNMENT_H
#define _I386_MALLOC_ALIGNMENT_H

#include <stddef.h>

/* INTERNAL_SIZE_T is the word-size used for internal bookkeeping of
   chunk sizes.  See sysdeps/generic/malloc-alignment.h for details.  */
#ifndef INTERNAL_SIZE_T
# define INTERNAL_SIZE_T size_t
#endif

/* The corresponding word size.  */
#define SIZE_SZ (sizeof (INTERNAL_SIZE_T))

#define MALLOC_ALIGNMENT 16

/* The corresponding bit mask value.  */
#define MALLOC_ALIGN_MASK (MALLOC_ALIGNMENT - 1)

#endif /* !defined(_I386_MALLOC_ALIGNMENT_H) */
