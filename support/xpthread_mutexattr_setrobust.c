/* pthread_mutexattr_setrobust with error checking.
   Copyright (C) 2016-2025 Free Software Foundation, Inc.
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

#include <support/xthread.h>

void
xpthread_mutexattr_setrobust (pthread_mutexattr_t *attr, int flag)
{
  xpthread_check_return ("pthread_mutexattr_setrobust",
                         pthread_mutexattr_setrobust (attr, flag));
}
