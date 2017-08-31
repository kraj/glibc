/* qsort, sort an array.
   This file is part of the GNU C Library.
   Copyright (C) 2018 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   GLicense as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* The qsort{_r} implementation is based on Smoothsort by Dijkstra [1]
   with the optimization to used O(1) extra space (implicit Leonardo
   Heaps).  A extensive analysis on algorithm details was written by
   Keith Schwarz [2].  This implementation is based on paper [3].

   The Smoothsort is used because:

   1. follows the comparison sort with an average-case lower bound of
      O(n * log (n)).

   2. It uses O(1) extra memory (the implicit Leonardo heap plus stack
      function usage).  It allows the function to be as-safe and also
      to avoid extra latency (due a possible malloc or extra function
      calls on a mergesort for instance).

   3. It has a the advantage over heapsort to approximate of O(n) for
      inputs already sorted in some degree.

   [1] http://www.cs.utexas.edu/users/EWD/ewd07xx/EWD796a.PDF
   [2] http://www.keithschwarz.com/smoothsort/
   [3] http://www.enterag.ch/hartwig/order/smoothsort.pdf
 */

#include <alloca.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Helper to get the address of BASE access as an array of elements of
   size SIZE.  */
static inline void *
arr (void *base, size_t idx, size_t size)
{
  return (void*)((uintptr_t)base + (idx * size));
}

/* Bitvector to encode the used Leonardo Heaps.  We need at most 1.7i bits
   to encode all the Leonardo numbers that can fit in a machine with word
   of size 2^i.  */
typedef size_t lnbitvector_t[2];

static inline void
incr (lnbitvector_t p)
{
  size_t r = p[0] + 1;
  p[1] += ((p[0] ^ r) & p[0]) >> (sizeof(size_t) * 8 - 1);
  p[0] = r;
}

static inline void
decr (lnbitvector_t p)
{
  size_t r = p[0] - 1;
  p[1] -= ((r ^ p[0]) & r) >> (sizeof(size_t) * 8 - 1);
  p[0] = r;
}

static inline void
shr (lnbitvector_t p)
{
  p[0] >>= 1;
  p[0] |= p[1] << (sizeof(size_t) * 8 - 1);
  p[1] >>= 1;
}

static inline void
shl (lnbitvector_t p)
{
  p[1] <<= 1;
  p[1] |= p[0] >> (sizeof(size_t) * 8 - 1);
  p[0] <<= 1;
}


/* Swap SIZE bytes between addresses A and B.  Helper to generic types
   are provided as an optimization.  */

typedef void (*swap_t)(void *, void *, size_t);

static inline bool
check_alignment (const void *base, size_t align)
{
  return _STRING_ARCH_unaligned || ((uintptr_t)base % (align - 1)) == 0;
}

static void
swap_u32 (void *a, void *b, size_t size)
{
  uint32_t tmp = *(uint32_t*) a;
  *(uint32_t*) a = *(uint32_t*) b;
  *(uint32_t*) b = tmp;
}

static void
swap_u64 (void *a, void *b, size_t size)
{
  uint64_t tmp = *(uint64_t*) a;
  *(uint64_t*) a = *(uint64_t*) b;
  *(uint64_t*) b = tmp;
}

static inline void
swap_generic (void *a, void *b, size_t size)
{
  unsigned char tmp[256];
  do
    {
      size_t s = size > sizeof (tmp) ? sizeof (tmp) : size;
      memcpy (tmp, a, s);
      a = __mempcpy (a, b, s);
      b = __mempcpy (b, tmp, s);
      size -= s;
    }
  while (size > 0);
}

static inline swap_t
select_swap_func (const void *base, size_t size)
{
  if (size == 4 && check_alignment (base, 4))
    return swap_u32;
  else if (size == 8 && check_alignment (base, 8))
    return swap_u64;
  return swap_generic;
}


struct state_t
{
  size_t q;         /* Length of the unsorted prefix (1 <= q <= n).  */
  size_t n;         /* Total number of elements.  */
  size_t r;
  size_t b;         /* Leonardo number.  */
  size_t c;         /* Next Leonardo number.  */
  lnbitvector_t p;  /* Bitvector to track the Leonardo heaps.  */
};

static inline size_t
up (struct state_t *s)
{
  shr (s->p);

  size_t next = s->b + s->c + 1;
  s->c = s->b;
  s->b = next;
  return next;
}

static inline size_t
down (struct state_t *s, size_t bit)
{
  shl (s->p);

  s->p[0] |= bit;

  size_t prev = s->c;
  s->c = s->b - s->c - 1;
  s->b = prev;
  return prev;
}

/* Invariant arguments used generic functions.  */
struct args_t
{
  void *m;            /* Base pointer to array.  */
  size_t s;           /* Element size.  */
  __compar_fn_t cmp;  /* Comparison function.  */
  swap_t swap;        /* Swap function.  */
};

#include "qsort_common.c"

void
qsort (void *base, size_t nmemb, size_t size, __compar_fn_t cmp)
{
  struct args_t args = { base, size, cmp, select_swap_func (base, size) };
  struct state_t s = { 1, nmemb, 0, 1, 1, { 1, 0 } };

  if (nmemb <= 0)
    return;

  buildtree (&args, &s);

  trinkle (&args, s.r, s);

  buildsorted (&args, &s);
}
libc_hidden_def (qsort)

struct args_t_r
{
  void *m;
  size_t s;
  __compar_d_fn_t cmp;
  void *a;
  swap_t swap;
};

#define R_VERSION
#include "qsort_common.c"

void
__qsort_r (void *base, size_t nmemb, size_t size, __compar_d_fn_t cmp,
	   void *arg)
{
  struct args_t_r args = { base, size, cmp, arg, select_swap_func (base, size) };
  struct state_t s = { 1, nmemb, 0, 1, 1, { 1, 0 } };

  if (nmemb <= 1)
    return;

  buildtree_r (&args, &s);

  trinkle_r (&args, s.r, s);

  buildsorted_r (&args, &s);
}

libc_hidden_def (__qsort_r)
weak_alias (__qsort_r, qsort_r)
