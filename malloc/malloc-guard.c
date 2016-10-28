/* Heap protector variables.
   Copyright (C) 2016 Free Software Foundation, Inc.
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
   not, see <http://www.gnu.org/licenses/>.  */

/* These variables are defined in a separate file because the static
   startup code initializes them, but this should not pull the rest of
   the libc malloc implementation into the link.  */

#include <malloc-internal.h>

/* The heap cookie.  The lowest three bits (corresponding to
   SIZE_BITS) in __malloc_guard_header must be clear.  Initialized
   during libc startup, and computed by elf/dl-keysetup.c.  */
INTERNAL_SIZE_T __malloc_header_guard; /* For size.  */
INTERNAL_SIZE_T __malloc_footer_guard; /* For prev_size.  */
