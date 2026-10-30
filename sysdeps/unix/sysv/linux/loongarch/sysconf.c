/* Copyright (C) 2026 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <unistd.h>
#include <limits.h>

static long int linux_sysconf (int name);

#define L1_IU_PRESENT (1U << 0)
#define L1_D_PRESENT  (1U << 2)
#define L2_IU_PRESENT (1U << 3)
#define L2_IU_UNIFY   (1U << 4)
#define L3_IU_PRESENT (1U << 10)
#define L3_IU_UNIFY   (1U << 11)

#define L2_IU_UNIFIED_PRESENT (L2_IU_PRESENT | L2_IU_UNIFY)
#define L3_IU_UNIFIED_PRESENT (L3_IU_PRESENT | L3_IU_UNIFY)

static inline long int
get_cache_associativity (unsigned int value)
{
  /* bit[15:0]: number of ways minus 1.  */
  return (value & 0x0000ffff) + 1;
}

static inline long int
get_cache_sets (unsigned int value)
{
  /* bit[23:16]: log2 of the number of sets.  */
  unsigned int shift = ((value & 0x00ff0000) >> 16);
  if (shift >= LONG_WIDTH - 1)
    return 0;

  return 1L << shift;
}

static inline long int
get_cache_linesize (unsigned int value)
{
  /* bit[30:24]: log2 of the cache line size.  */
  unsigned int shift = ((value & 0x7f000000) >> 24);
  if (shift == 0 || shift >= LONG_WIDTH - 1)
    return 0;

  return 1L << shift;
}

static long int
cache_info (int name, int first, unsigned int word)
{
  long int size = 0;
  unsigned int value = __builtin_loongarch_cpucfg (word);
  long int ways = get_cache_associativity (value);
  long int sets = get_cache_sets (value);
  long int linesz = get_cache_linesize (value);

  /* Report no information if decoding the number of sets or the line size
     from the configuration word fails.  */
  if (sets == 0 || linesz == 0)
    return 0;

  /* Calculate the cache size, report no information if it overflows.  */
  if (__builtin_mul_overflow (ways, sets, &size)
      || __builtin_mul_overflow (size, linesz, &size))
    return 0;

  switch (name - first)
    {
    case 0:
      return size;
    case 1:
      return ways;
    default:
      return linesz;
    }
}

/* Get the value of the system variable NAME.  */
long int
__sysconf (int name)
{
  if (name < _SC_LEVEL1_ICACHE_SIZE || name > _SC_LEVEL4_CACHE_LINESIZE)
    return linux_sysconf (name);

  /* LoongArch doesn't have L4 caches.  */
  if (name >= _SC_LEVEL4_CACHE_SIZE)
    return 0;

  unsigned int config = __builtin_loongarch_cpucfg (0x10);

  switch (name)
    {
    case _SC_LEVEL1_ICACHE_SIZE:
    case _SC_LEVEL1_ICACHE_ASSOC:
    case _SC_LEVEL1_ICACHE_LINESIZE:
      {
	if ((config & L1_IU_PRESENT) == 0)
	  return 0;

	return cache_info (name, _SC_LEVEL1_ICACHE_SIZE, 0x11);
      }
    case _SC_LEVEL1_DCACHE_SIZE:
    case _SC_LEVEL1_DCACHE_ASSOC:
    case _SC_LEVEL1_DCACHE_LINESIZE:
      {
	if ((config & L1_D_PRESENT) == 0)
	  return 0;

	return cache_info (name, _SC_LEVEL1_DCACHE_SIZE, 0x12);
      }
    case _SC_LEVEL2_CACHE_SIZE:
    case _SC_LEVEL2_CACHE_ASSOC:
    case _SC_LEVEL2_CACHE_LINESIZE:
      {
	/* This implementation supports only a unified L2 cache, CPUCFG 0x13 is
	   decoded as its cache descriptor.  Split L2 configurations require
	   separate handling, and are currently unsupported.  */
	if ((config & L2_IU_UNIFIED_PRESENT) != L2_IU_UNIFIED_PRESENT)
	  return 0;

	return cache_info (name, _SC_LEVEL2_CACHE_SIZE, 0x13);
      }
    case _SC_LEVEL3_CACHE_SIZE:
    case _SC_LEVEL3_CACHE_ASSOC:
    case _SC_LEVEL3_CACHE_LINESIZE:
      {
	/* This implementation supports only a unified L3 cache, CPUCFG 0x14 is
	   decoded as its cache descriptor.  Split L3 configurations require
	   separate handling, and are currently unsupported.  */
	if ((config & L3_IU_UNIFIED_PRESENT) != L3_IU_UNIFIED_PRESENT)
	  return 0;

	return cache_info (name, _SC_LEVEL3_CACHE_SIZE, 0x14);
      }
    default:
      __builtin_unreachable ();
    }
}

/* Now the generic Linux version.  */
#undef __sysconf
#define __sysconf static linux_sysconf
#include <sysdeps/unix/sysv/linux/sysconf.c>
