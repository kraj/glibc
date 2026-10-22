/* Check that fts fails with ENAMETOOLONG, instead of returning corrupted
   paths, when the paths get longer than USHRT_MAX (BZ 34782).
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

#define BASENAME "tst-fts-bz34782."
/* Enough levels for the paths to get longer than USHRT_MAX.  */
#define NESTED_DEPTH 300
#include "tst-fts-bz22944.c"
