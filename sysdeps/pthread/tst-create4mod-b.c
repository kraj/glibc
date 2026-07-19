/* DSO B for tst-create4: writes 'B' into the shared buffer several
   times, yielding between writes to maximize visible interleaving.
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

#include <stdatomic.h>
#include <sched.h>

#include "tst-create4.h"

static void __attribute__ ((constructor))
init_b (void)
{
  for (int i = 0; i < TST_CREATE4_NSTEPS; ++i)
    {
      int idx = atomic_fetch_add_explicit (&tst_create4_seq_idx, 1,
					   memory_order_relaxed);
      tst_create4_seq_buf[idx] = 'B';
      sched_yield ();
    }
}
