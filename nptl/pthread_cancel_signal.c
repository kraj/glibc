/* Signal handling for pthread cancellation.
   Copyright (C) 2002-2026 Free Software Foundation, Inc.
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
#include <stdint.h>
#include <sys/ucontext.h>
#include <cancellation-pc-check.h>
#include "pthreadP.h"
#include <unistd.h>

/* For asynchronous cancellation we use a signal.  */
static void
sigcancel_handler (int sig, siginfo_t *si, void *ctx)
{
  if (sig != SIGCANCEL)
    return;

  /* SIGTIMER aliases SIGCANCEL.  A timer expiration (SI_TIMER) delivered here
     means the SIGEV_THREAD helper thread is running a notification function
     with SIGTIMER unblocked (see timer_create.c): the expiration cannot be
     serviced now, so it is ignored and recorded as an overrun that the
     application can observe with timer_getoverrun.  si_overrun accounts for
     any further expirations the kernel folded into this one.  */
  if (si->si_code == SI_TIMER)
    {
      struct pthread *self = THREAD_SELF;
      atomic_fetch_add_relaxed (&self->timer_overrun, 1 + si->si_overrun);
      return;
    }

  /* Safety check.  It would be possible to call this function for
     other signals and send a signal from another process.  This is not
     correct and might even be a security problem.  Try to catch as
     many incorrect invocations as possible.  */
  if (si->si_pid != __getpid()
      || si->si_code != SI_TKILL)
    return;

  /* Check if asynchronous cancellation mode is set and cancellation is not
     already in progress, or if interrupted instruction pointer falls within
     the cancellable syscall bridge.
     For interruptable syscalls with external side-effects (i.e. partial
     reads), the kernel will set the IP to after __syscall_cancel_arch_end,
     thus disabling the cancellation and allowing the process to handle such
     conditions.  */
  struct pthread *self = THREAD_SELF;
  int oldval = atomic_load_relaxed (&self->cancelhandling);
  if (cancel_enabled_and_canceled_and_async (oldval)
      || cancellation_pc_check (ctx))
    __syscall_do_cancel ();
}

/* Install the SIGCANCEL handler if it has not been installed yet.  This is
   done lazily from __pthread_cancel, and eagerly from the POSIX timer code
   (which unblocks SIGCANCEL/SIGTIMER in the SIGEV_THREAD helper thread and
   therefore needs the handler in place before any signal can be delivered).  */
void
__pthread_install_sigcancel_handler (void)
{
  static int init_sigcancel = 0;
  if (atomic_load_relaxed (&init_sigcancel) == 0)
    {
      struct sigaction sa;
      sa.sa_sigaction = sigcancel_handler;
      /* The signal handle should be non-interruptible to avoid the risk of
	 spurious EINTR caused by SIGCANCEL sent to process or if
	 pthread_cancel() is called while cancellation is disabled in the
	 target thread.  */
      sa.sa_flags = SA_SIGINFO | SA_RESTART;
      __sigemptyset (&sa.sa_mask);
      __libc_sigaction (SIGCANCEL, &sa, NULL);
      atomic_store_relaxed (&init_sigcancel, 1);
    }
}
