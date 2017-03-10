/* Allocation from a fixed-size buffer.
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

/* TODOs:

   More documentation, including usage scenarios:
     (1) NSS code (with struct scratch_buffer), although this style is
         not a good idea (callee should malloc if caller does not know
         size).
     (2) Compute allocation size of a buffer (without overflow
         checking), allocate out_buffer, overflow checking while
         writing to the buffer.  Timezone parser (time/tzfile.c)
         contains a particularly interesting example.

   out_buffer_init should be out_buffer_create and return the struct.

   Add out_buffer_allocate which calls malloc (and returns the struct,
   to help aliasing analysis).

   Fix invalid pointer handling.  All pointers should point into the
   buffer.  Use NULL for __OUT_BUFFER_INVALID_POINTER.  Make NULL
   argument to out_buffer_init invalid, to resolve the
   out_buffer_get_bytes ambiguity.

   Add byte array copying functions.

   Add string copying functions.

   Maybe add string formatting functions.
 */

#ifndef _OUT_BUFFER_H
#define _OUT_BUFFER_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/param.h>

struct out_buffer
{
  uintptr_t __out_buffer_current;
  uintptr_t __out_buffer_end;
};

enum
  {
    __OUT_BUFFER_INVALID_POINTER = 1,
  };

static inline void
out_buffer_init (struct out_buffer *buf, void *start, size_t size)
{
  if (size > 0)
    {
      buf->__out_buffer_current = (uintptr_t) start;
      buf->__out_buffer_end = buf->__out_buffer_current + size;
    }
  else
    {
      /* Avoid storing __OUT_BUFFER_INVALID_POINTER (marking the
	 buffer as failed) for a zero-length buffer.  */
      buf->__out_buffer_current = 0;
      buf->__out_buffer_end = 0;
    }
}

static __always_inline void *
out_buffer_current (struct out_buffer *buf)
{
  return (void *) buf->__out_buffer_current;
}

/* Internal function.  Return the remaining number of bytes in the
   buffer.  */
static __always_inline size_t
__out_buffer_remaining (const struct out_buffer *buf)
{
  return buf->__out_buffer_end - buf->__out_buffer_current;
}

/* Internal function.  Mark the buffer as failed. */
static inline void
out_buffer_mark_failed (struct out_buffer *buf)
{
  buf->__out_buffer_current = __OUT_BUFFER_INVALID_POINTER;
  buf->__out_buffer_end = __OUT_BUFFER_INVALID_POINTER;
}

/* Return true if the buffer has been marked as failed.  */
static inline bool
out_buffer_has_failed (const struct out_buffer *buf)
{
  return buf->__out_buffer_current == __OUT_BUFFER_INVALID_POINTER;
}

/* Add a single byte to the buffer (consuming the space for this
   byte).  Mark the buffer as failed if there is not enough room.  */
static inline void
out_buffer_add_byte (struct out_buffer *buf, unsigned char b)
{
  if (__glibc_likely (buf->__out_buffer_current < buf->__out_buffer_end))
    {
      *(unsigned char *) buf->__out_buffer_current = b;
      ++buf->__out_buffer_current;
    }
  else
   out_buffer_mark_failed (buf);
}

/* Obtain a pointer to LENGTH bytes in BUF, and consume these bytes.
   NULL is returned if there is not enough room, and the buffer is
   marked as failed, or if the buffer has already failed.  */
static inline void *
out_buffer_get_bytes (struct out_buffer *buf, size_t length)
{
  if (__glibc_likely (length > 0))
    {
      if (length <= __out_buffer_remaining (buf))
	{
	  void *result = (void *) buf->__out_buffer_current;
	  buf->__out_buffer_current += length;
	  return result;
	}
      else
	{
	  out_buffer_mark_failed (buf);
	  return NULL;
	}
    }
  else if (out_buffer_has_failed (buf))
    return NULL;
  else
    /* Non-null pointer to empty array.  */
    return (void *) __OUT_BUFFER_INVALID_POINTER;
}

/* Internal function.  Statically assert that the type size is
   constant and valid.  */
static __always_inline size_t
__out_buffer_assert_size (size_t size)
{
  if (!__builtin_constant_p (size))
    {
      __errordecl (error, "type size is not constant");
      error ();
    }
  else if (size == 0)
    {
      __errordecl (error, "type size is zero");
      error ();
    }
  return size;
}

/* Internal function.  Statically assert that the type alignment is
   constant and valid.  */
static __always_inline size_t
__out_buffer_assert_align (size_t align)
{
  if (!__builtin_constant_p (align))
    {
      __errordecl (error, "type alignment is not constant");
      error ();
    }
  else if (align == 0)
    {
      __errordecl (error, "type alignment is zero");
      error ();
    }
  else if (!powerof2 (align))
    {
      __errordecl (error, "type alignment is not a power of two");
      error ();
    }
  return align;
}

/* Internal function.  Obtain a pointer to an object.  */
static inline void *
__out_buffer_get (struct out_buffer *buf, size_t size, size_t align)
{
  if (size == 1 && align == 1)
    return out_buffer_get_bytes (buf, size);

  size_t current = buf->__out_buffer_current;
  size_t aligned = roundup (current, align);
  size_t new_current = aligned + size;
  if (aligned >= current        /* No overflow in align step.  */
      && new_current >= size    /* No overflow in size computation.  */
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

/* Obtain a TYPE * pointer to an object in BUF of TYPE.  Consume these
   bytes from the buffer.  Return NULL and mark the buffer as failed
   if if there is not enough room in the buffer, or if the buffer has
   failed before.  */
#define out_buffer_get(buf, type)                       \
  ((type *) __out_buffer_get                            \
   (buf, __out_buffer_assert_size (sizeof (type)),      \
    __out_buffer_assert_align (__alignof__ (type))))


/* Internal function.  Allocate an array.  COUNT_LIMIT is used
   internally to check for multiplication overflow without a
   division.  */
void * __libc_out_buffer_get_array (struct out_buffer *buf,
				    size_t size, size_t align,
				    size_t count, size_t count_limit)
  internal_function;
libc_hidden_proto (__libc_out_buffer_get_array)

/* Obtain a TYPE * pointer to an array of COUNT objects in BUF of
   TYPE.  Consume these bytes from the buffer.  Return NULL and mark
   the buffer as failed if if there is not enough room in the buffer,
   or if the buffer has failed before.  */
#define out_buffer_get_array(buf, type, count)          \
  ((type *) __libc_out_buffer_get_array			\
   (buf, __out_buffer_assert_size (sizeof (type)),      \
    __out_buffer_assert_align (__alignof__ (type)),     \
    count, ((size_t) -1) / sizeof (type)))

#endif /* _OUT_BUFFER_H */
