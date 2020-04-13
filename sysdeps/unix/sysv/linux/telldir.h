/* Linux internal telldir definitions.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

#ifndef _TELLDIR_H
#define _TELLDIR_H 1

#ifndef __LP64__

/* On platforms where long int is smaller than off64_t this is how the
   returned value is encoded and returned by 'telldir'.  If the directory
   offset can be enconded in 31 bits it is returned in the 'info' member
   with 'is_packed' set to 1.

   Otherwise, the 'info' member describes an index in a dynamic array at
   'DIR' structure.  */

union dirstream_packed
{
  long int l;
  struct
  {
    unsigned long is_packed:1;
    unsigned long info:31;
  } p;
};

_Static_assert (sizeof (long int) == sizeof (union dirstream_packed),
		"sizeof (long int) != sizeof (union dirstream_packed)");

/* telldir will mantain a list of offsets that describe the obtained diretory
   position if it can fit this information in the returned 'dirstream_packed'
   struct.  */

struct dirstream_loc
{
  off64_t filepos;
};

# define DYNARRAY_STRUCT  dirstream_loc_t
# define DYNARRAY_ELEMENT struct dirstream_loc
# define DYNARRAY_PREFIX  dirstream_loc_
# include <malloc/dynarray-skeleton.c>
#else

_Static_assert (sizeof (long int) == sizeof (off64_t),
		"sizeof (long int) != sizeof (off64_t)");
#endif /* __LP64__  */

#endif /* _TELLDIR_H  */
