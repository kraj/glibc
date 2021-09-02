/* Copyright (C) 1991-2021 Free Software Foundation, Inc.
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

/* If you consider tuning this algorithm, you should consult first:
   Engineering a sort function; Jon Bentley and M. Douglas McIlroy;
   Software - Practice and Experience; Vol. 23 (11), 1249-1265, 1993.  */

#include <alloca.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Swap SIZE bytes between addresses A and B.  These helpers are provided
   along the generic one as an optimization.  */

typedef void (*swap_func_t)(void * restrict, void * restrict, size_t);

/* Return trues is elements can be copied used word load and sortes.
   The size must be a multiple of the alignment, and the base address.  */
static inline bool
is_aligned_to_copy (const void *base, size_t size, size_t align)
{
  unsigned char lsbits = size;
#if !_STRING_ARCH_unaligned
  lsbits |= (unsigned char)(uintptr_t) base;
#endif
  return (lsbits & (align - 1)) == 0;
}

#define SWAP_WORDS_64 (swap_func_t)0
#define SWAP_WORDS_32 (swap_func_t)1
#define SWAP_BYTES    (swap_func_t)2

static void
swap_words_64 (void * restrict a, void * restrict b, size_t n)
{
  do
   {
     n -= 8;
     uint64_t t = *(uint64_t *)(a + n);
     *(uint64_t *)(a + n) = *(uint64_t *)(b + n);
     *(uint64_t *)(b + n) = t;
   } while (n);
}

static void
swap_words_32 (void * restrict a, void * restrict b, size_t n)
{
  do
   {
     n -= 4;
     uint32_t t = *(uint32_t *)(a + n);
     *(uint32_t *)(a + n) = *(uint32_t *)(b + n);
     *(uint32_t *)(b + n) = t;
   } while (n);
}

static void
swap_bytes (void * restrict a, void * restrict b, size_t n)
{
  /* Use multiple small memcpys with constant size to enable inlining
     on most targets.  */
  enum { SWAP_GENERIC_SIZE = 32 };
  unsigned char tmp[SWAP_GENERIC_SIZE];
  while (n > SWAP_GENERIC_SIZE)
    {
      memcpy (tmp, a, SWAP_GENERIC_SIZE);
      a = memcpy (a, b, SWAP_GENERIC_SIZE) + SWAP_GENERIC_SIZE;
      b = memcpy (b, tmp, SWAP_GENERIC_SIZE) + SWAP_GENERIC_SIZE;
      n -= SWAP_GENERIC_SIZE;
    }
  memcpy (tmp, a, n);
  memcpy (a, b, n);
  memcpy (b, tmp, n);
}

/* Replace the indirect call with a serie of if statements.  It should help
   the branch predictor.  */
static void
do_swap (void * restrict a, void * restrict b, size_t size,
	 swap_func_t swap_func)
{
  if (swap_func == SWAP_WORDS_64)
    swap_words_64 (a, b, size);
  else if (swap_func == SWAP_WORDS_32)
    swap_words_32 (a, b, size);
  else
    swap_bytes (a, b, size);
}

/* Discontinue quicksort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 4

/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct
  {
    char *lo;
    char *hi;
    size_t depth;
  } stack_node;

/* The stack needs log (total_elements) entries (we could even subtract
   log(MAX_THRESH)).  Since total_elements has type size_t, we get as
   upper bound for log (total_elements):
   bits per byte (CHAR_BIT) * sizeof(size_t).  */
enum { STACK_SIZE = CHAR_BIT * sizeof (size_t) };

static inline stack_node *
push (stack_node *top, char *lo, char *hi, size_t depth)
{
  top->lo = lo;
  top->hi = hi;
  top->depth = depth;
  return ++top;
}

static inline stack_node *
pop (stack_node *top, char **lo, char **hi, size_t *depth)
{
  --top;
  *lo = top->lo;
  *hi = top->hi;
  *depth = top->depth;
  return top;
}


/* A fast, small, non-recursive O(nlog n) heapsort, adapted from Linux
   lib/sort.c.  Used on introsort implementation as a fallback routine with
   worst-case performance of O(nlog n) and worst-case space complexity of
   O(1).  */

static inline size_t
parent (size_t i, unsigned int lsbit, size_t size)
{
  i -= size;
  i -= size & -(i & lsbit);
  return i / 2;
}

static void
heapsort_r (void *base, void *end, size_t size, swap_func_t swap_func,
	    __compar_d_fn_t cmp, void *arg)
{
  size_t num = ((uintptr_t) end - (uintptr_t) base) / size;
  size_t n = num * size, a = (num/2) * size;
  /* Used to find parent  */
  const unsigned int lsbit = size & -size;

  /* num < 2 || size == 0.  */
  if (a == 0)
    return;

  for (;;)
    {
      size_t b, c, d;

      if (a != 0)
	/* Building heap: sift down --a */
	a -= size;
      else if (n -= size)
	/* Sorting: Extract root to --n */
	do_swap (base, base + n, size, swap_func);
      else
	break;

      /* Sift element at "a" down into heap.  This is the "bottom-up" variant,
	 which significantly reduces calls to cmp_func(): we find the sift-down
	 path all the way to the leaves (one compare per level), then backtrack
	 to find where to insert the target element.

	 Because elements tend to sift down close to the leaves, this uses fewer
	 compares than doing two per level on the way down.  (A bit more than
	 half as many on average, 3/4 worst-case.).  */
      for (b = a; c = 2 * b + size, (d = c + size) < n;)
	b = cmp (base + c, base + d, arg) >= 0 ? c : d;
      if (d == n)
	/* Special case last leaf with no sibling.  */
	b = c;

      /* Now backtrack from "b" to the correct location for "a".  */
      while (b != a && cmp (base + a, base + b, arg) >= 0)
	b = parent (b, lsbit, size);
      /* Where "a" belongs.  */
      c = b;
      while (b != a)
	{
	  /* Shift it into place.  */
	  b = parent (b, lsbit, size);
          do_swap (base + b, base + c, size, swap_func);
        }
    }
}

/* Order size using quicksort.  This implementation incorporates
   four optimizations discussed in Sedgewick:

   1. Non-recursive, using an explicit stack of pointer that store the
      next array partition to sort.  To save time, this maximum amount
      of space required to store an array of SIZE_MAX is allocated on the
      stack.  Assuming a 32-bit (64 bit) integer for size_t, this needs
      only 32 * sizeof(stack_node) == 256 bytes (for 64 bit: 1024 bytes).
      Pretty cheap, actually.

   2. Chose the pivot element using a median-of-three decision tree.
      This reduces the probability of selecting a bad pivot value and
      eliminates certain extraneous comparisons.

   3. Only quicksorts TOTAL_ELEMS / MAX_THRESH partitions, leaving
      insertion sort to order the MAX_THRESH items within each partition.
      This is a big win, since insertion sort is faster for small, mostly
      sorted array segments.

   4. The larger of the two sub-partitions is always pushed onto the
      stack first, with the algorithm then concentrating on the
      smaller partition.  This *guarantees* no more than log (total_elems)
      stack size is needed (actually O(1) in this case)!  */

static void
insertion_sort (void *const pbase, size_t total_elems, size_t size,
                swap_func_t swap_func,
	        __compar_d_fn_t cmp, void *arg)
{
  char *base_ptr = (char *) pbase;
  char *const end_ptr = &base_ptr[size * (total_elems - 1)];
  char *tmp_ptr = base_ptr;
#define min(x, y) ((x) < (y) ? (x) : (y))
  const size_t max_thresh = MAX_THRESH * size;
  char *thresh = min(end_ptr, base_ptr + max_thresh);
  char *run_ptr;

  /* Find smallest element in first threshold and place it at the
     array's beginning.  This is the smallest array element,
     and the operation speeds up insertion sort's inner loop. */

  for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
    if (cmp (run_ptr, tmp_ptr, arg) < 0)
      tmp_ptr = run_ptr;

  if (tmp_ptr != base_ptr)
    do_swap (tmp_ptr, base_ptr, size, swap_func);

  /* Insertion sort, running from left-hand-side up to right-hand-side.  */

  run_ptr = base_ptr + size;
  while ((run_ptr += size) <= end_ptr)
    {
      tmp_ptr = run_ptr - size;
      while (cmp (run_ptr, tmp_ptr, arg) < 0)
        tmp_ptr -= size;

      tmp_ptr += size;
      if (tmp_ptr != run_ptr)
        {
          char *trav;

          trav = run_ptr + size;
          while (--trav >= run_ptr)
            {
              char c = *trav;
              char *hi, *lo;

              for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
                *hi = *lo;
              *hi = c;
            }
        }
    }
}

void
_quicksort (void *const pbase, size_t total_elems, size_t size,
	    __compar_d_fn_t cmp, void *arg)
{
  char *base_ptr = (char *) pbase;

  const size_t max_thresh = MAX_THRESH * size;

  if (total_elems <= 1)
    /* Avoid lossage with unsigned arithmetic below.  */
    return;

  swap_func_t swap_func;
  if (is_aligned_to_copy (pbase, size, 8))
    swap_func = SWAP_WORDS_64;
  else if (is_aligned_to_copy (pbase, size, 4))
    swap_func = SWAP_WORDS_32;
  else
    swap_func = SWAP_BYTES;

  /* Maximum depth before quicksort switches to heapsort.  */
  size_t depth = 2 * (sizeof (size_t) * CHAR_BIT - 1
		      - __builtin_clzl (total_elems));

  if (total_elems > MAX_THRESH)
    {
      char *lo = base_ptr;
      char *hi = &lo[size * (total_elems - 1)];
      stack_node stack[STACK_SIZE];
      stack_node *top = stack;

      top = push (top, NULL, NULL, depth);

      while (stack < top)
        {
	  if (depth == 0)
	    {
	      heapsort_r (lo, hi, size, swap_func, cmp, arg);
              top = pop (top, &lo, &hi, &depth);
	      continue;
	    }

          char *left_ptr;
          char *right_ptr;

	  /* Select median value from among LO, MID, and HI. Rearrange
	     LO and HI so the three values are sorted. This lowers the
	     probability of picking a pathological pivot value and
	     skips a comparison for both the LEFT_PTR and RIGHT_PTR in
	     the while loops. */

	  char *mid = lo + size * ((hi - lo) / size >> 1);

	  if ((*cmp) ((void *) mid, (void *) lo, arg) < 0)
	    do_swap (mid, lo, size, swap_func);
	  if ((*cmp) ((void *) hi, (void *) mid, arg) < 0)
	    do_swap (mid, hi, size, swap_func);
	  else
	    goto jump_over;
	  if ((*cmp) ((void *) mid, (void *) lo, arg) < 0)
	    do_swap (mid, lo, size, swap_func);
	jump_over:;

	  left_ptr  = lo + size;
	  right_ptr = hi - size;

	  /* Here's the famous ``collapse the walls'' section of quicksort.
	     Gotta like those tight inner loops!  They are the main reason
	     that this algorithm runs much faster than others. */
	  do
	    {
	      while ((*cmp) ((void *) left_ptr, (void *) mid, arg) < 0)
		left_ptr += size;

	      while ((*cmp) ((void *) mid, (void *) right_ptr, arg) < 0)
		right_ptr -= size;

	      if (left_ptr < right_ptr)
		{
		  do_swap (left_ptr, right_ptr, size, swap_func);
		  if (mid == left_ptr)
		    mid = right_ptr;
		  else if (mid == right_ptr)
		    mid = left_ptr;
		  left_ptr += size;
		  right_ptr -= size;
		}
	      else if (left_ptr == right_ptr)
		{
		  left_ptr += size;
		  right_ptr -= size;
		  break;
		}
	    }
	  while (left_ptr <= right_ptr);

          /* Set up pointers for next iteration.  First determine whether
             left and right partitions are below the threshold size.  If so,
             ignore one or both.  Otherwise, push the larger partition's
             bounds on the stack and continue sorting the smaller one. */

          if ((size_t) (right_ptr - lo) <= max_thresh)
            {
              if ((size_t) (hi - left_ptr) <= max_thresh)
		/* Ignore both small partitions. */
		top = pop (top, &lo, &hi, &depth);
              else
		/* Ignore small left partition. */
                lo = left_ptr;
            }
          else if ((size_t) (hi - left_ptr) <= max_thresh)
	    /* Ignore small right partition. */
            hi = right_ptr;
          else if ((right_ptr - lo) > (hi - left_ptr))
            {
	      /* Push larger left partition indices. */
	      top = push (top, lo, right_ptr, depth - 1);
              lo = left_ptr;
            }
          else
            {
	      /* Push larger right partition indices. */
	      top = push (top, left_ptr, hi, depth - 1);
              hi = right_ptr;
            }
        }
    }

  /* Once the BASE_PTR array is partially sorted by quicksort the rest
     is completely sorted using insertion sort, since this is efficient
     for partitions below MAX_THRESH size. BASE_PTR points to the beginning
     of the array to sort, and END_PTR points at the very last element in
     the array (*not* one beyond it!). */
  insertion_sort (pbase, total_elems, size, swap_func, cmp, arg);
}
