/* Basic tests for sealing.  Static version.
   Copyright (C) 2024 Free Software Foundation, Inc.
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

/* This test checks the GNU_PROPERTY_MEMORY_SEAL handling on a statically
   built binary.  In this case only the vDSO (if existent) will be sealed.  */

#define TEST_STATIC              1

/* Expected libraries that loader will seal.  */
static const char *expected_sealed_vmas[] =
{
  "",
};

/* Expected non sealed libraries.  */
static const char *expected_non_sealed_vmas[] =
{
  "tst-dl_mseal-static-noseal",
  /* Auxiary pages mapped by the kernel.  */
  "[vdso]",
  "[sigpage]",
};

/* Auxiliary kernel pages where permission can not be changed.  */
static const char *expected_non_sealed_special[] =
{
  "[vectors]",
};

#include "tst-dl_mseal-skeleton.c"
