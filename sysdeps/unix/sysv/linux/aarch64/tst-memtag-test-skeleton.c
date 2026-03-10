/* Tests skeleton for MEMTAG tests.
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

#include <stdlib.h>
#include <libc-diag.h>
#include <support/check.h>
#include <support/xsignal.h>
#include <support/xthread.h>
#include <unistd.h>

#include "tst-mte-helper.h"

static void
sigsegv_handler (int signum, siginfo_t *si, void *context)
{
  if (si->si_signo == SIGSEGV
      && (si->si_code == SEGV_MTESERR || si->si_code == SEGV_MTEAERR))
    _exit (EXIT_MTESERR);
  else
    _exit (EXIT_FAILURE);
}

/* Prevent inlining so the stack frame is definitively constructed to
   trigger a stack frame, and optimization to avoid compiler optimize
   away the invalid stack operation.  */
static void *
__attribute_noinline__
__attribute_optimization_barrier__
trigger_mte_fault (void *closure)
{
  DIAG_PUSH_NEEDS_COMMENT;
  DIAG_IGNORE_NEEDS_COMMENT_GCC (16, "-Warray-bounds");

  sigset_t mask;
  sigemptyset (&mask);
  sigaddset (&mask, SIGSEGV);

  /* Aling to a MTE tag granule.  */
  _Alignas (16) char stack_buffer[16];
  volatile char *ptr = &stack_buffer[16];

  *(volatile char *)ptr;

  return NULL;
}

void
run_mte_test (void *closure)
{
  test_mode mode = (test_mode)(uintptr_t)closure;

  {
    struct sigaction sa = {
      .sa_sigaction = sigsegv_handler,
      .sa_flags = SA_NODEFER | SA_SIGINFO,
    };
    sigemptyset (&sa.sa_mask);
    xsigaction (SIGSEGV, &sa, NULL);
  }

  if (mode == TEST_MAIN)
    trigger_mte_fault (NULL);
  else
    {
      pthread_t t = xpthread_create (NULL, trigger_mte_fault, NULL);
      xpthread_join (t);
    }

  _exit (EXIT_FAILURE);
}
