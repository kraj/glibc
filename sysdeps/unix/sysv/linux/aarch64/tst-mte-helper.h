/* Helper routines to check MEMTAG support.
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

#include <stdlib.h>
#include <stdbool.h>
#include <sys/prctl.h>
#include <support/xsignal.h>
#include <support/check.h>

typedef enum
{
  TEST_MAIN = 0,
  TEST_THREAD = 1,
} test_mode;

#define EXIT_MTESERR 79

void run_mte_test (void *);

static inline bool
mte_enable (void)
{
  int ctrl = prctl (PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
  return ctrl > 0 && (ctrl & PR_MTE_TCF_MASK) != PR_MTE_TCF_NONE;
}

static inline int
mte_mode (void)
{
  int ctrl = prctl (PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
  TEST_VERIFY_EXIT (ctrl >= 0);
  return ctrl & PR_MTE_TCF_MASK;
}

#define __attribute_disable_mte_stack__ \
 __attribute__((no_sanitize("memtag-stack")))

#define MTE_GRANULE_SIZE 16

static void
__attribute_maybe_unused__
sigsegv_handler_expected (int signum, siginfo_t *si, void *context)
{
  if (si->si_signo == SIGSEGV
      && (si->si_code == SEGV_MTESERR || si->si_code == SEGV_MTEAERR))
    _exit (EXIT_MTESERR);
  else
    _exit (EXIT_FAILURE);
}

static void
__attribute_maybe_unused__
sigsegv_handler_failure (int signum, siginfo_t *si, void *context)
{
  support_record_failure ();
  _exit (EXIT_FAILURE);
}

static inline void
install_sigsegv_handler (void)
{
  {
    struct sigaction sa = {
      .sa_sigaction = sigsegv_handler_expected,
      .sa_flags = SA_NODEFER | SA_SIGINFO,
    };
    sigemptyset (&sa.sa_mask);
    xsigaction (SIGSEGV, &sa, NULL);
  }
}

static inline void
install_sigsegv_handler_failure (void)
{
  {
    struct sigaction sa = {
      .sa_sigaction = sigsegv_handler_failure,
      .sa_flags = SA_NODEFER | SA_SIGINFO,
    };
    sigemptyset (&sa.sa_mask);
    xsigaction (SIGSEGV, &sa, NULL);
  }
}

#endif
