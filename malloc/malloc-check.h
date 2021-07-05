/* glibc.malloc.check function interface for libc_malloc_debug.so.
   Copyright (C) 2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#ifndef _MALLOC_CHECK_H_
# define _MALLOC_CHECK_H_

# if !IS_IN (libc_malloc_debug)
#  error "These functions must only be used in libc_malloc_debug.so"
# else
extern size_t __malloc_usable_size (void *);
extern size_t __malloc_check_malloc_usable_size (void *);
extern bool __malloc_check_malloc (size_t, void **);
extern bool __malloc_check_free (void *);
extern bool __malloc_check_realloc (void *, size_t, void **);
extern bool __malloc_check_memalign (size_t, size_t, void **);
# endif
#endif
