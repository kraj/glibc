/* Module for tst-origin-secure (the attacker-controlled copy).
   Copyright (C) 2026 Free Software Foundation, Inc.
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

#include "tst-origin-secure.h"

/* If the victim reports this copy, the loader opened the un-normalized rpath
   and resolved it through the attacker's symlink -- i.e. the trusted-path
   check was bypassed (bug 34360).  */
int
origin_secure_id (void)
{
  return ORIGIN_SECURE_ID_ATTACKER;
}
