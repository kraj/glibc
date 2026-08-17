/* Victim program for tst-origin-secure.
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

extern int __libc_enable_secure;

/* Report both which module was loaded and whether the loader ran in secure
   mode, so the driver can tell a genuine trusted-path bypass apart from a run
   that simply was not secure.

   The exit status is the combination of the ORIGIN_SECURE_STATUS_* bits:

     _NONE                    trusted copy, not secure
     _ATTACKER                attacker copy, not secure  (the control run)
     _SECURE                  trusted copy, secure       (a fixed loader)
     _SECURE | _ATTACKER      attacker copy, secure      (the bug: the raw
                                                          rpath was opened)  */
int
main (void)
{
  int status = ORIGIN_SECURE_STATUS_NONE;
  if (origin_secure_id () == ORIGIN_SECURE_ID_ATTACKER)
    status |= ORIGIN_SECURE_STATUS_ATTACKER;
  if (__libc_enable_secure != 0)
    status |= ORIGIN_SECURE_STATUS_SECURE;
  return status;
}
