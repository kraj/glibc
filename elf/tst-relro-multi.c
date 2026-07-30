/* Multiple PT_GNU_RELRO test.
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

#include <support/check.h>
#include <support/check_mem_access.h>

/* The tst-elf-edit.py --note-to-relro convert relro_a and relro_b
   to page-padded PT_GNU_RELRO seguments.  */
extern unsigned long int relro_a;
extern unsigned long int gap_d;
extern unsigned long int relro_b;

static int
do_test (void)
{
  /* Both RELRO regions must be readable but not writable.  */
  TEST_COMPARE (check_mem_access (&relro_a, false), true);
  TEST_COMPARE (check_mem_access (&relro_a, true), false);
  TEST_COMPARE (check_mem_access (&relro_b, false), true);
  TEST_COMPARE (check_mem_access (&relro_b, true), false);

  /* The gap between the two RELRO regions must remain writable.  */
  TEST_COMPARE (check_mem_access (&gap_d, true), true);

  return 0;
}

#include <support/test-driver.c>
