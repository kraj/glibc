/* AArch64 tests for heap memory tagging.
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
#include <support/support.h>
#include <support/xsignal.h>
#include <support/test-driver.h>
#include <array_length.h>

#include "tst-mte-helper.h"

/* Characteristic malloc sizes to cover various allocation methods.  */
size_t sizes[] = {
  1,
  16, 40, 64, 120,
  128, 500, 1000,
  1050, 4096, 5000, 65000,
  131072, 2000000
};

static void check_malloc (size_t len)
{
  printf ("testing malloc with req size %zu\n", len);
  void *tm = malloc (len);
  if (!check_tags (tm))
    printf ("tagged pointer?: %016lx\n", (uintptr_t)tm);
  free (tm);
}

static void check_calloc (size_t len)
{
  size_t num = len / sizeof (uint64_t) + 1;
  printf ("testing calloc with req size %zu\n", num * sizeof (uint64_t));
  uint64_t *tm = calloc (num, sizeof (uint64_t));
  if (check_tags (tm))
    for (int n = 0; n < num; n ++)
      TEST_VERIFY_EXIT (tm[n] == 0);
  else
    printf ("tagged pointer?: %016lx\n", (uintptr_t)tm);
  free (tm);
}

static void check_memalign (size_t len, size_t alignment)
{
  printf ("testing memalign(%zu) with req size %zu\n", alignment, len);
  void *tm = memalign (alignment, len);
  if (!check_tags (tm))
    printf ("tagged pointer?: %016lx\n", (uintptr_t)tm);
  free (tm);
}

static void check_valloc (size_t len)
{
  printf ("testing valloc with req size %zu\n", len);
  void *tm = valloc (len);
  if (!check_tags (tm))
    printf ("tagged pointer?: %016lx\n", (uintptr_t)tm);
  free_sized (tm, len);
}

static void check_pvalloc (size_t len)
{
  printf ("testing pvalloc with req size %zu\n", len);
  void *tm = pvalloc (len);
  if (!check_tags (tm))
    printf ("tagged pointer?: %016lx\n", (uintptr_t)tm);
  free_sized (tm, len);
}

static void check_posix_memalign (size_t len, size_t alignment)
{
  printf ("testing posix_memalign(%zu) with req size %zu\n", alignment, len);
  void *p = NULL;
  int err = posix_memalign (&p, alignment, len);
  if (err)
    perror ("posix_memalign");
  TEST_VERIFY (p != NULL);
  TEST_VERIFY (err == 0);
  if (!check_tags (p))
    printf ("tagged pointer?: %016lx\n", (uintptr_t)p);
  free_aligned_sized (p, alignment, len);
}

static void check_aligned_alloc (size_t len, size_t alignment)
{
  printf ("testing aligned_alloc(%zu) with req size %zu\n", alignment, len);
  void *tm = aligned_alloc (alignment, len);
  if (!check_tags (tm))
    printf ("tagged pointer?: %016lx\n", (uintptr_t)tm);
  free_aligned_sized (tm, alignment, len);
}

static int
do_test (void)
{
  /* Check if MTE is supported, configured and enabled.  */
  check_mte_enabled ();

  array_foreach_const (plen, sizes)
    check_malloc (*plen);

  array_foreach_const (plen, sizes)
    check_calloc (*plen);

  array_foreach_const (plen, sizes)
    {
      check_memalign (*plen, 2);
      check_memalign (*plen, 4);
      check_memalign (*plen, 8);
      check_memalign (*plen, 16);
      check_memalign (*plen, 32);
    }

  array_foreach_const (plen, sizes)
    check_valloc (*plen);

  array_foreach_const (plen, sizes)
    check_pvalloc (*plen);

  array_foreach_const (plen, sizes)
    {
      check_posix_memalign (*plen, sizeof (void *) * 1);
      check_posix_memalign (*plen, sizeof (void *) * 2);
      check_posix_memalign (*plen, sizeof (void *) * 4);
    }

  array_foreach_const (plen, sizes)
    {
      check_aligned_alloc (*plen, 2);
      check_aligned_alloc (*plen, 4);
      check_aligned_alloc (*plen, 8);
      check_aligned_alloc (*plen, 16);
      check_aligned_alloc (*plen, 32);
    }

  return 0;
}

#include <support/test-driver.c>
