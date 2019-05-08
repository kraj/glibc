/* Copyright (C) 1997-2019 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <signal.h>
#include <string.h>

#include <sysdep.h>
#include <sigsetops.h>
#include <sys/syscall.h>

/* New ports should not define the obsolete SA_RESTORER, however some
   architecture requires for compat mode and/or due old ABI.  */
#include <kernel_sigaction.h>

#ifndef SA_RESTORER
# define SET_SA_RESTORER(kact, act)
# define RESET_SA_RESTORER(act, kact)
#endif

/* SPARC passes the restore function as an argument to rt_sigaction.  */
#ifndef STUB
# define STUB(act, sigsetsize) (sigsetsize)
#endif

static sigset_t handler_set;

void __get_sighandler_set (sigset_t *set)
{
  *set = handler_set;
}

/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__libc_sigaction (int sig, const struct sigaction *act, struct sigaction *oact)
{
  int result;

  struct kernel_sigaction kact, koact;

  if (act)
    {
      /* Tracks which signal had a signal handler set different from default
	 (SIG_DFL/SIG_IGN).  It allows optimize posix_spawn to reset only
	 those signals.  It might incur in false positive, since it not easy
	 to remove bits from the mask without race conditions, but it does not
	 allow false negative since the mask is updated atomically prior the
	 syscall.  The false positive incur in just extra sigactions on
	 posix_spawn.  */
      if (act->sa_handler != SIG_DFL && act->sa_handler != SIG_IGN)
	__sigaddset_atomic (&handler_set, sig);
      kact.k_sa_handler = act->sa_handler;
      memcpy (&kact.sa_mask, &act->sa_mask, sizeof (sigset_t));
      kact.sa_flags = act->sa_flags;
      SET_SA_RESTORER (&kact, act);
    }

  /* XXX The size argument hopefully will have to be changed to the
     real size of the user-level sigset_t.  */
  result = INLINE_SYSCALL_CALL (rt_sigaction, sig,
				act ? &kact : NULL,
				oact ? &koact : NULL, STUB (act, _NSIG / 8));

  if (oact && result >= 0)
    {
      oact->sa_handler = koact.k_sa_handler;
      memcpy (&oact->sa_mask, &koact.sa_mask, sizeof (sigset_t));
      oact->sa_flags = koact.sa_flags;
      RESET_SA_RESTORER (oact, &koact);
    }
  return result;
}
libc_hidden_def (__libc_sigaction)

#include <nptl/sigaction.c>
