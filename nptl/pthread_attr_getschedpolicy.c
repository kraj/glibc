/* Copyright (C) 2002-2025 Free Software Foundation, Inc.
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

#include "pthreadP.h"


int
__pthread_attr_getschedpolicy (const pthread_attr_t *attr, int *policy)
{
  struct pthread_attr *iattr;

  iattr = (struct pthread_attr *) attr;

  /* Store the current values.  */
  *policy = iattr->schedpolicy;

  return 0;
}
strong_alias (__pthread_attr_getschedpolicy, pthread_attr_getschedpolicy)
