/* Specialized dynarray for C strings.
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

/* This file provides a dynamic C string with an initial stack allocated
   buffer.  Since it is based on dynarray, it provided dynamic size
   expansion and heap usage for large strings.

   The following parameters are optional:

   CHAR_ARRAY_INITIAL_SIZE
      The size of the statically allocated array (default is 256).  It will
      be used to define DYNARRAY_INITIAL_SIZE.

   The following functions are provided:

   bool char_array_init_empty (struct char_array *);
   bool char_array_init_str (struct char_array *, const char *);
   bool char_array_init_str_size (struct char_array *, const char *, size_t);
   bool char_array_is_empty (struct char_array *);
   const char *char_array_str (struct char_array *);
   char char_array_pos (struct char_array *, size_t);
   size_t char_array_length (struct char_array *);
   bool char_array_set_str (struct char_array *, const char *);
   bool char_array_set_str_size (struct char_array *, const char *, size_t);
   void char_array_erase (struct char_array *, size_t);
   bool char_array_crop (struct char_array *, size_t);
   bool char_array_prepend_str (struct char_array *, const char *);
   bool char_array_append_str (struct char_array *, const char *);
   bool char_array_replace_str_pos (struct char_array *, size_t, const char *,
				    size_t);

   For instance:

   struct char_array str;
   // str == "testing";
   char_array_init_str (&str, "testing");
   // c == 's'
   char c = char_array_pos (&str, 2);
   // str = "testing2";
   char_array_set_str (&str, "testing2");
   // str = "testi";
   char_array_erase (&str, 5);
   // str = "123testi";
   char_array_prepend_str (&str, "123");
   // len = 8;
   size_t len = char_array_length (&str);
   // str = "123testi456";
   char_array_append_str (&str, "456");
   // str = "123testi789";
   char_array_replace_str_pos (&str, 7, "789", 3);
 */

#define DYNARRAY_STRUCT            char_array
#define DYNARRAY_ELEMENT           char
#define DYNARRAY_PREFIX            char_array_
#ifndef CHAR_ARRAY_INITIAL_SIZE
# define CHAR_ARRAY_INITIAL_SIZE 256
#endif
#define DYNARRAY_INITIAL_SIZE  CHAR_ARRAY_INITIAL_SIZE
#include <malloc/dynarray-skeleton.c>

#include <malloc/malloc-internal.h>
#include <malloc/char_array.h>

/* Return a const char for the internal C string handled by 'array'.  */
__attribute__ ((unused, nonnull (1)))
static const char *
char_array_str (struct char_array *array)
{
  return char_array_begin (array);
}

/* Return the character at position 'pos' from the char_array 'array'.  */
__attribute__ ((unused, nonnull (1)))
static char
char_array_pos (struct char_array *array, size_t pos)
{
  return *char_array_at (array, pos);
}

/* Calculate the length of the string, excluding the terminating null.  */
__attribute__ ((unused, nonnull (1)))
static size_t
char_array_length (struct char_array *array)
{
  /* Exclude the final '\0'.  */
  return array->dynarray_header.used - 1;
}

/* Copy up 'size' bytes from string 'str' to char_array 'array'.  A final
   '\0' is appended in the char_array.  */
__attribute__ ((unused, nonnull (1, 2)))
static bool
char_array_set_str_size (struct char_array *array, const char *str,
			 size_t size)
{
  size_t newsize;
  if (check_add_overflow_size_t (size, 1, &newsize))
    __libc_dynarray_overflow_failure (size, 1);

  if (!char_array_resize (array, newsize))
    return false;

  __char_array_set_str_size (&array->dynarray_abstract, str, size);
  return true;
}

/* Copy the contents of string 'str' to char_array 'array', including the
   final '\0'.  */
__attribute__ ((unused, nonnull (1, 2)))
static bool
char_array_set_str (struct char_array *array, const char *str)
{
  return char_array_set_str_size (array, str, strlen (str));
}

/* Initialize the char_array 'array' and sets it to an empty string ("").  */
__attribute__ ((unused, nonnull (1)))
static bool
char_array_init_empty (struct char_array *array)
{
  char_array_init (array);
  return char_array_set_str (array, "");
}

/* Initialize the char_array 'array' and copy the content of string 'str'.  */
__attribute__ ((unused, nonnull (1, 2)))
static bool
char_array_init_str (struct char_array *array, const char *str)
{
  char_array_init (array);
  return char_array_set_str (array, str);
}

/* Initialize the char_array 'array' and copy the content of string 'str'
   up to 'size' characteres.  */
__attribute__ ((unused, nonnull (1, 2)))
static bool
char_array_init_str_size (struct char_array *array, const char *str,
			  size_t size)
{
  char_array_init (array);
  return char_array_set_str_size (array, str, size);
}

/* Return if the char_array contain any characteres.  */
__attribute__ ((unused, nonnull (1)))
static bool
char_array_is_empty (struct char_array *array)
{
  return *char_array_begin (array) == '\0';
}

/* Remove the byte at position 'pos' from char_array 'array'.  The contents
   are moved internally if the position is not at the end of the internal
   buffer.  */
__attribute__ ((unused, nonnull (1)))
static bool
char_array_erase (struct char_array *array, size_t pos)
{
  if (pos >= array->dynarray_header.used - 1)
    return false;

  __char_array_erase (&array->dynarray_abstract, pos);
  return true;
}

/* Resize the char_array 'array' to size 'count' maintaining the ending
   '\0' byte.  */
__attribute__ ((unused, nonnull (1)))
static bool
char_array_crop (struct char_array *array, size_t size)
{
  if (size >= (array->dynarray_header.used - 1)
      || !char_array_resize (array, size + 1))
    return false;

  array->dynarray_header.array[size] = '\0';
  return true;
}

/* Prepend the contents of string 'str' to char_array 'array', including the
   final '\0' byte.  */
__attribute__ ((unused, nonnull (1, 2)))
static bool
char_array_prepend_str (struct char_array *array, const char *str)
{
  size_t size = strlen (str);
  /* Resizing the array might change its used elements and we need below
     to correct copy the elements.  */
  size_t used = array->dynarray_header.used;

  size_t newsize;
  if (check_add_overflow_size_t (used, size, &newsize))
    __libc_dynarray_overflow_failure (used, size);

  /* Make room for the string and copy it.  */
  if (!char_array_resize (array, newsize))
    return false;
  __char_array_prepend_str_size (&array->dynarray_abstract, str, size, used);
  return true;
}

/* Append the contents of string 'str' to char_array 'array, including the
   final '\0' byte.  */
__attribute__ ((unused, nonnull (1, 2)))
static bool
char_array_append_str (struct char_array *array, const char *str)
{
  size_t size = strlen (str);
  /* Resizing the array might change its used elements and it used it below
     to correct copy the elements.  */
  size_t used = array->dynarray_header.used - 1;

  /* 'used' does account for final '\0', so there is no need to add
     an extra element to calculate the final required size.  */
  size_t newsize;
  if (check_add_overflow_size_t (used + 1, size, &newsize))
    __libc_dynarray_overflow_failure (used + 1, size);

  if (!char_array_resize (array, newsize))
    return false;

  /* Start to append at '\0' up to string length and add a final '\0'.  */
  *(char*) mempcpy (array->dynarray_header.array + used, str, size) = '\0';
  return true;
}

/* Replace the contents starting of position 'pos' of char_array 'array'
   with the contents of string 'str' up to 'len' bytes.  A final '\0'
   is appended in the string.  */
__attribute__ ((unused, nonnull (1, 3)))
static bool
char_array_replace_str_pos (struct char_array *array, size_t pos,
                            const char *str, size_t len)
{
  if (pos > array->dynarray_header.used)
    __libc_dynarray_at_failure (array->dynarray_header.used, pos);

  size_t newsize;
  if (check_add_overflow_size_t (pos, len, &newsize)
      || check_add_overflow_size_t (newsize, 1, &newsize)
      || !char_array_resize (array, newsize))
    return false;

  __char_array_replace_str_pos (&array->dynarray_abstract, pos, str, len);
  return true;
}
