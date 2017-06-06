/* Specialized dynarray for C strings.  Implementation file.
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

#include <malloc/char_array.h>

void
__char_array_set_str_size (struct dynarray_header *header, const char *str,
			   size_t size)
{
  *((char *) mempcpy (header->array, str, size)) = '\0';
  header->used = size + 1;
}
libc_hidden_def (__char_array_set_str_size)

void
__char_array_erase (struct dynarray_header *header, size_t pos)
{
  char *ppos = header->array + pos;
  char *lpos = header->array + header->used;
  ptrdiff_t size = lpos - ppos;
  memmove (ppos, ppos + 1, size);
  header->used--;
}
libc_hidden_def (__char_array_erase)

void
__char_array_prepend_str_size (struct dynarray_header *header,
			       const char *str, size_t size, size_t used)
{
  memmove (header->array + size, header->array, used);
  memcpy (header->array, str, size);
}
libc_hidden_def (__char_array_prepend_str_size)

void
__char_array_replace_str_pos (struct dynarray_header *header, size_t pos,
			      const char *str, size_t len)
{
  char *start = header->array + pos;
  *(char *) mempcpy (start, str, len) = '\0';
}
libc_hidden_def (__char_array_replace_str_pos)
