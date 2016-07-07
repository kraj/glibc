/* Definitions of functions to access `dev_t' values.
   Copyright (C) 2003-2016 Free Software Foundation, Inc.
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

#include <features.h>
#undef __USE_EXTERN_INLINES
#include <sys/sysmacros.h>
#include <sys/types.h>

unsigned int
gnu_dev_major (dev_t dev)
{
  __major_body (dev);
}

unsigned int
gnu_dev_minor (dev_t dev)
{
  __minor_body (dev);
}

dev_t
gnu_dev_makedev (unsigned int major, unsigned int minor)
{
  __makedev_body (major, minor);
}
