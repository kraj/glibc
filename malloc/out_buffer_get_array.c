/* Array allocation from a fixed-size buffer.
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

#include <out_buffer.h>

void *
internal_function
__libc_out_buffer_get_array (struct out_buffer *buf,
                             size_t element_size, size_t align,
                             size_t count, size_t count_limit)
{
  /* Avoid spurious success if count == 0.  */
  if (out_buffer_has_failed (buf))
    return NULL;

  size_t current = buf->__out_buffer_current;
  /* The caller asserts that align is a power of two.  */
  size_t aligned = (current + align - 1) & ~(align - 1);
  size_t size = element_size * count;
  size_t new_current = aligned + size;
  if (count <= count_limit     /* Multiplication did not overflow.  */
      && aligned >= current    /* No overflow in align step.  */
      && new_current >= size   /* No overflow in size computation.  */
      && new_current <= buf->__out_buffer_end) /* Room in buffer.  */
    {
      buf->__out_buffer_current = new_current;
      return (void *) aligned;
    }
  else
    {
      out_buffer_mark_failed (buf);
      return NULL;
    }
}
libc_hidden_def (__libc_out_buffer_get_array)
