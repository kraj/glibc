/* Test that pthread_getattr_np accounts for the stack guard gap.
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

#include <inttypes.h>
#include <pthread.h>
#include <stackinfo.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xstdio.h>
#include <support/xthread.h>
#include <support/xunistd.h>

#if _STACK_GROWS_DOWN
static bool
stack_vma (uintptr_t *lo, uintptr_t *hi)
{
  FILE *f = xfopen ("/proc/self/maps", "r");
  char *line = NULL;
  size_t len = 0;
  bool found = false;
  while (xgetline (&line, &len, f) > 0)
    if (strstr (line, "[stack]") != NULL
	&& sscanf (line, "%" SCNxPTR "-%" SCNxPTR, lo, hi) == 2)
      {
	found = true;
	break;
      }
  free (line);
  xfclose (f);
  return found;
}
#endif

static int
do_test (void)
{
#if !_STACK_GROWS_DOWN
  FAIL_UNSUPPORTED ("the stack does not grow down");
#else
  support_need_proc ("Reads /proc/self/maps to get the stack vma.");

  uintptr_t lo, hi;
  if (!stack_vma (&lo, &hi))
    FAIL_UNSUPPORTED ("no [stack] in /proc/self/maps");

  long pagesz = xsysconf (_SC_PAGESIZE);
  uintptr_t planted = (lo - 100 * 1024) & -(uintptr_t) pagesz;
  void *m = mmap ((void *) planted, pagesz, PROT_READ | PROT_WRITE,
		  MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
  if (m == MAP_FAILED)
    FAIL_UNSUPPORTED ("cannot map below the stack: %m");
  if ((uintptr_t) m != planted)
    {
      /* MAP_FIXED_NOREPLACE was treated as a hint.  */
      xmunmap (m, pagesz);
      FAIL_UNSUPPORTED ("MAP_FIXED_NOREPLACE not supported");
    }

  pthread_attr_t attr;
  TEST_COMPARE (pthread_getattr_np (pthread_self (), &attr), 0);
  void *stackaddr;
  size_t stacksize;
  TEST_COMPARE (pthread_attr_getstack (&attr, &stackaddr, &stacksize), 0);
  xpthread_attr_destroy (&attr);

  /* The vma may only have grown down since it was read.  */
  uintptr_t lo2, hi2;
  TEST_VERIFY (stack_vma (&lo2, &hi2));
  TEST_VERIFY (lo2 <= (uintptr_t) stackaddr && (uintptr_t) stackaddr <= lo);
  TEST_VERIFY ((uintptr_t) stackaddr > planted + pagesz);

  return 0;
#endif
}

#include <support/test-driver.c>
