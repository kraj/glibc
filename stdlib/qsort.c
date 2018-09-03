/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Douglas C. Schmidt (schmidt@ics.uci.edu).

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
   <http://www.gnu.org/licenses/>.  */

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Swap SIZE bytes between addresses A and B.  These helpers are provided
   along the generic one as an optimization.  */

typedef void (*swap_t)(void *, void *, size_t);

static inline bool
check_alignment (const void *base, size_t align)
{
  return _STRING_ARCH_unaligned || ((uintptr_t)base & (align - 1)) == 0;
}

static void
swap_u32 (void * restrict a, void * restrict b, size_t size)
{
  uint32_t *ua = a, *ub = b, tmp = *ua;
  *ua = *ub, *ub = tmp;
}

static void
swap_u64 (void * restrict a, void * restrict b, size_t size)
{
  uint64_t *ua = a, *ub = b, tmp = *ua;
  *ua = *ub, *ub = tmp;
}

static void
swap_generic (void * restrict a, void * restrict b, size_t size)
{
  /* Use multiple small memcpys with constant size to enable inlining
     on most targets.  */
  enum {
    SWAP_GENERIC_SIZE = 32
  };
  unsigned char tmp[SWAP_GENERIC_SIZE];
  while (size > SWAP_GENERIC_SIZE)
    {
      memcpy (tmp, a, SWAP_GENERIC_SIZE);
      a = memcpy (a, b, SWAP_GENERIC_SIZE) + SWAP_GENERIC_SIZE;
      b = memcpy (b, tmp, SWAP_GENERIC_SIZE) + SWAP_GENERIC_SIZE;
      size -= SWAP_GENERIC_SIZE;
    }
  memcpy (tmp, a, size);
  memcpy (a, b, size);
  memcpy (b, tmp, size);
}

static inline swap_t
select_swap_func (const void *base, size_t size)
{
  if (size == sizeof (uint32_t)
      && check_alignment (base, _Alignof (uint32_t)))
    return swap_u32;
  else if (size == sizeof (uint64_t)
	   && check_alignment (base, _Alignof (uint64_t)))
    return swap_u64;
  return swap_generic;
}

/* Discontinue quicksort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 4

/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct
  {
    char *lo;
    char *hi;
  } stack_node;

/* The next 4 #defines implement a very fast in-line stack abstraction. */
/* The stack needs log (total_elements) entries (we could even subtract
   log(MAX_THRESH)).  Since total_elements has type size_t, we get as
   upper bound for log (total_elements):
   bits per byte (CHAR_BIT) * sizeof(size_t).  */
#define STACK_SIZE	(CHAR_BIT * sizeof(size_t))
#define PUSH(low, high)	((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define	POP(low, high)	((void) (--top, (low = top->lo), (high = top->hi)))
#define	STACK_NOT_EMPTY	(stack < top)

/* Helper function to calculate the maximum depth before quicksort switches
   to heapsort.  X should be higher than 0.  */
static inline size_t
lg (size_t x)
{
  return sizeof (size_t) * CHAR_BIT - 1 - __builtin_clzl(x);
}

#define R_VERSION
#define R_FUNC    __qsort_r
#define R_HEAP    heapsort_r
#include <stdlib/qsort_common.c>

libc_hidden_def (__qsort_r)
weak_alias (__qsort_r, qsort_r)

#define R_FUNC   qsort
#define R_HEAP   heapsort
#include <stdlib/qsort_common.c>

libc_hidden_def (qsort)
