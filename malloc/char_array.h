/* Specialized dynarray for C strings.  Shared definitions.
   Copyright (C) 2017 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#ifndef _CHAR_ARRAY_H
#define _CHAR_ARRAY_H

#include <malloc/dynarray.h>

/* Internal funciton.  Set the dynarray to the content of the string STR up
   to SIZE bytes.  The dynarray must be resized previously.  */
void __char_array_set_str_size (struct dynarray_header *, const char *str,
				size_t size);

/* Internal function.  Remove the character at position POS from dynarray.
   The position must be a valid one.  */
void __char_array_erase (struct dynarray_header *, size_t pos);

/* Internal function.  Prepend the content of string STR up to SIZE bytes to
   dynarray by moving USED bytes forward.  The dynarray must be resized
   previously.  */
void __char_array_prepend_str_size (struct dynarray_header *,
				    const char *str, size_t size,
				    size_t used);

/* Internal function.  Replace the content of dynarray starting at position
   POS with the content of string STR up to LEN bytes.  The dynarray must
   be resize previously and STR must contain at least LEN bytes.  */
void __char_array_replace_str_pos (struct dynarray_header *, size_t pos,
				   const char *str, size_t len);

#ifndef _ISOMAC
libc_hidden_proto (__char_array_set_str_size)
libc_hidden_proto (__char_array_erase)
libc_hidden_proto (__char_array_prepend_str_size)
libc_hidden_proto (__char_array_replace_str_pos)
#endif

#endif
