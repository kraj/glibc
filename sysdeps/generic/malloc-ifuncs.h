/* Definitions for ifunc resolvers for malloc: generic version.
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

#ifndef _GENERIC_MALLOC_IFUNCS_H
#define _GENERIC_MALLOC_IFUNCS_H

/* Targets should define this macro if they provide ifuncs for
   malloc functions.  When USE_MULTIARCH_MALLOC is defined, the
   following functions should be implemented via ifuncs:

     malloc, calloc, free, realloc
     memalign, valloc, pvalloc
     posix_memalign
     malloc_usable_size
     aligned_alloc, free_sized, free_aligned_sized

*/
#if defined(USE_MULTIARCH_MALLOC) || !defined(USE_MULTIARCH)
# undef USE_MULTIARCH_MALLOC
#endif

#endif /* GENERIC_MALLOC_IFUNCS_H */
