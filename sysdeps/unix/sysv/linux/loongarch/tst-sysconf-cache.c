/* Test system cache parameters.
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

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <support/check.h>

/* Return true if N is a power of two (N > 0).  */
static bool
is_power_of_two (long int n)
{
  return n > 0 && (n & (n - 1)) == 0;
}

/* Check that the size, assoc and linesize values of one cache level are
   either all absent, or all present and mutually consistent:
     - linesize is a power of two
     - assoc is positive
     - size is a multiple of assoc
     - the resulting number of sets (size / (assoc * linesize)) is a power of
     two, matching the CPUCFG encoding.
   Currently must_be_present is true for L1 caches.  */

static void
check_cache_level (const char *level, int size_sc, int assoc_sc,
		   int linesize_sc, bool must_be_present)
{
  long int size = sysconf (size_sc);
  long int assoc = sysconf (assoc_sc);
  long int linesize = sysconf (linesize_sc);
  printf ("Test sysconf (_SC_%s_*)\n", level);

  if (size == 0 && assoc == 0 && linesize == 0)
    {
      if (must_be_present)
	FAIL_UNSUPPORTED ("size, assoc and linesize are reported absent");
    }
  else if (size > 0 && assoc > 0 && linesize > 0)
    {
      long int way_size = size / assoc;
      long int sets = way_size / linesize;

      TEST_VERIFY (is_power_of_two (linesize));
      TEST_COMPARE (size % assoc, 0);
      TEST_COMPARE (way_size % linesize, 0);
      TEST_VERIFY (is_power_of_two (sets));
    }
  else
    FAIL ("inconsistent result (size = %ld, assoc = %ld, linesize = %ld)",
	  size, assoc, linesize);
}

static void
check_cache_level_absent (const char *level, int size_sc, int assoc_sc,
			  int linesize_sc)
{
  printf ("Test sysconf (_SC_%s_*) absent\n", level);
  TEST_COMPARE (sysconf (size_sc), 0);
  TEST_COMPARE (sysconf (assoc_sc), 0);
  TEST_COMPARE (sysconf (linesize_sc), 0);
}

static int
do_test (void)
{
  check_cache_level ("LEVEL1_ICACHE", _SC_LEVEL1_ICACHE_SIZE,
		     _SC_LEVEL1_ICACHE_ASSOC, _SC_LEVEL1_ICACHE_LINESIZE,
		     true);
  check_cache_level ("LEVEL1_DCACHE", _SC_LEVEL1_DCACHE_SIZE,
		     _SC_LEVEL1_DCACHE_ASSOC, _SC_LEVEL1_DCACHE_LINESIZE,
		     true);
  check_cache_level ("LEVEL2_CACHE", _SC_LEVEL2_CACHE_SIZE,
		     _SC_LEVEL2_CACHE_ASSOC, _SC_LEVEL2_CACHE_LINESIZE, false);
  check_cache_level ("LEVEL3_CACHE", _SC_LEVEL3_CACHE_SIZE,
		     _SC_LEVEL3_CACHE_ASSOC, _SC_LEVEL3_CACHE_LINESIZE, false);
  check_cache_level_absent ("LEVEL4_CACHE", _SC_LEVEL4_CACHE_SIZE,
			    _SC_LEVEL4_CACHE_ASSOC, _SC_LEVEL4_CACHE_LINESIZE);
  return 0;
}

#include <support/test-driver.c>
