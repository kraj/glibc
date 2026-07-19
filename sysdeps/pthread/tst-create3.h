/* Shared definitions for tst-create3 and tst-create3mod.
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

#ifndef _TST_CREATE3_H
#define _TST_CREATE3_H

/* Magic value published by the module constructor as its final action.
   Any dlopen caller that observes tst_create3mod_done with a different
   value immediately after dlopen returned has beaten the constructor.  */
#define TST_CREATE3_MAGIC_DONE 0xCAFEBABEu

#endif
