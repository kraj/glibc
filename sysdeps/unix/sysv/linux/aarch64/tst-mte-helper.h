/* AArch64 test helper functions for MTE.
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

#ifndef TST_MTE_HELPER_H
#define TST_MTE_HELPER_H

#include <support/check.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/auxv.h>
#include <sys/prctl.h>

#define GRANULE_SIZE 16

/* Extract logical tag from pointer PTR.  */
static __always_inline
uint64_t get_logical_tag (const void *ptr)
{
  uint64_t t = (uint64_t)ptr;
  return t >> 56ul & 0xf;
}

/* Load allocation tag from memory pointed-to by the PTR pointer.  */
static __always_inline
uint64_t get_allocation_tag (const void *ptr)
{
  uint64_t t;
  asm volatile ("ldg %0, [%1]" : "=r" (t) : "r" (ptr));
  return t >> 56ul & 0xf;
}

/* Read the Tag Check Override bit.  */
static __always_inline
uint64_t get_pstate_tco (void) {
  uint64_t t;
  asm volatile ("mrs %0, tco" : "=r" (t));
  return t;
}

static __always_inline
bool check_tags (void *tm)
{
  size_t len = malloc_usable_size (tm);
  TEST_VERIFY (len % GRANULE_SIZE == 0);

  uint64_t ltag = get_logical_tag (tm);
  TEST_VERIFY (ltag != 0);

  for (size_t offset = 0; offset < len; offset += GRANULE_SIZE)
    {
      const char *g = (char *)tm + offset;
      uint64_t atag = get_allocation_tag (g);
      TEST_COMPARE (ltag, atag);
      if (ltag != atag)
	{
          printf ("tagged ptr: %p usable size: %zu\n", tm, len);
	  printf ("tags mismatch at offset %zu: logical=%lu, allocation=%lu\n",
		  offset, ltag, atag);
          return false;
	}
    }
  return ltag != 0;
}

static __always_inline
void check_mte_enabled (void)
{
  /* Check if MTE is supported.  */
  if (!(getauxval (AT_HWCAP2) & HWCAP2_MTE))
    FAIL_UNSUPPORTED ("kernel or CPU does not support HWCAP2_MTE");

  /* Check if Tag Check Override bit is set.  */
  if (get_pstate_tco () != 0)
    FAIL_UNSUPPORTED ("MTE tag check override is enabled");

  /* Check applied MTE params.  */
  uint64_t x = (uint64_t) prctl (PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
  uint64_t status = (x & 1ul);
  uint64_t mode = (x & PR_MTE_TCF_MASK) >> PR_MTE_TCF_SHIFT;
  uint64_t tags = (x & PR_MTE_TAG_MASK) >> PR_MTE_TAG_SHIFT;

  printf ("MTE status: %4lx\n", status);
  printf ("MTE mode:   %4lx\n", mode);
  printf ("MTE tags:   %4lx\n", tags);

  /* This test should be run in sync mode for tag checks.  */
  TEST_VERIFY (status == 1);
  TEST_VERIFY (mode == PR_MTE_TCF_SYNC >> PR_MTE_TCF_SHIFT);
  TEST_VERIFY (tags == 0xfffe);
}

#endif // TST_MTE_HELPER_H
