/* Definitions shared by the tst-origin-secure test, its victim and modules.
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

#ifndef _TST_ORIGIN_SECURE_H
#define _TST_ORIGIN_SECURE_H 1

enum
  {
    ORIGIN_SECURE_ID_TRUSTED = 1,  /* The copy installed in the trusted
				      SLIBDIR.  */
    ORIGIN_SECURE_ID_ATTACKER = 2, /* The copy reachable only by resolving the
				      "sub" symlink.  */
  };

extern int origin_secure_id (void);

enum
  {
    ORIGIN_SECURE_STATUS_NONE = 0,
    ORIGIN_SECURE_STATUS_ATTACKER = 1 << 0,  /* The victim loaded attacker
						rather than the trusted.  */
    ORIGIN_SECURE_STATUS_SECURE = 1 << 1,    /* The loader ran the victim in
						secure mode.  */
  };

#endif
