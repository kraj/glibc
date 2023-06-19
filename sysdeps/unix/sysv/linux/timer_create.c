/* Copyright (C) 2003-2026 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <jmpbuf-unwind.h>
#include <kernel-posix-cpu-timers.h>
#include <kernel-posix-timers.h>
#include <ldsodefs.h>
#include <libc-diag.h>
#include <libc-internal.h>
#include <libc-lock.h>
#include <pthreadP.h>
#include <shlib-compat.h>

struct timer_helper_thread_args_t
{
  /* The barrier is used to synchronize the arguments copy from timer_create
     and the SIGEV_THREAD thread and to instruct the thread to exit if the
     timer_create syscall fails.  */
  pthread_barrier_t barrier;
  struct sigevent *evp;
};

static void *
timer_helper_thread (void *arg)
{
  struct pthread *self = THREAD_SELF;
  struct timer_helper_thread_args_t *args = arg;
  struct pthread_reset_cleanup_args_t clargs = {
    .cleanup_jmp_buf = self->cleanup_jmp_buf
  };

  void (*thrfunc) (sigval_t) = args->evp->sigev_notify_function;
  sigval_t sival = args->evp->sigev_value;

  __pthread_barrier_wait (&args->barrier);
  /* timer_create syscall failed.  */
  if (self->exiting)
    return 0;

  while (1)
    {
      siginfo_t si;
      while (__sigwaitinfo (&sigtimer_set, &si) < 0);

      if (si.si_code == SI_TIMER)
	{
	  /* Reset the overrun counter, further expirations that arrive while
	     the notification runs are delivered to the handler and added
	     there; timer_getoverrun reports the total.  */
	  atomic_store_relaxed (&self->timer_overrun, si.si_overrun);

	  /* POSIX requires SIGEV_THREAD notifications to behave as if a new
	     thread was created for each delivery.  Since the same helper
	     thread serves multiple firings, install a cancellation landing
	     pad rooted at this loop: if the notification function returns
	     normally, calls pthread_exit, or is cancelled, control unwinds
	     back here, the cleanup handler resets all observable per-thread
	     state - TLS destructors, TSD, libc thread-local state, DTV,
	     signal mask, and cancellation state - and the helper serves the
	     next firing instead of terminating.  State that must survive
	     across firings is intentionally preserved: the vDSO getrandom
	     buffer (opaque, forward-secure) and timerid (used to detect
	     timer_delete via its MSB after each firing).  */
	  struct pthread_unwind_buf cancel_buf;
	  DIAG_PUSH_NEEDS_COMMENT;
	  /* Same false-positive -Wstringop-overflow as in start_thread.  */
	  DIAG_IGNORE_NEEDS_COMMENT (11, "-Wstringop-overflow=");
	  int not_first_call
	    = setjmp ((struct __jmp_buf_tag *) cancel_buf.cancel_jmp_buf);
	  DIAG_POP_NEEDS_COMMENT;
	  cancel_buf.priv.data.prev = NULL;
	  cancel_buf.priv.data.cleanup = NULL;

	  if (__glibc_likely (! not_first_call))
	    {
	      self->cleanup_jmp_buf = &cancel_buf;
	      pthread_cleanup_push (__pthread_reset_state, &clargs);

	      /* Enable asynchronous cancellation.  A timer re-fire (SI_TIMER)
		 or the timer_delete wake (SI_QUEUE) reaching the handler is
		 ignored, so it neither cancels nor is queued.  */
	      internal_signal_unblock_signal (SIGTIMER);

	      thrfunc (sival);

	      pthread_cleanup_pop (1);
	    }
	}

      /* timer_delete sets the MSB and wakes this thread.  Relaxed MO is
	 sufficient because signaling this thread is a memory barrier.  */
      if (atomic_load_relaxed (&self->timerid) < 0)
	break;
    }

  return NULL;
}

/* Set up a SIGEV_THREAD timer: spawn the helper thread that will run the
   user's notify function on each firing, then create the kernel timer bound
   to that thread's TID.  Returns 0 on success and stores the resulting
   timer_t in *TIMERID; returns -1 with errno set on failure (from
   __pthread_attr_setsigmask_internal, __pthread_create, or the timer_create
   syscall).  ATTR is consumed but not destroyed by this function.  */
static int
timer_create_sigev_thread (clockid_t clockid, struct sigevent *evp,
			   timer_t *timerid, pthread_attr_t *attr)
{
  /* The helper thread unblocks SIGTIMER/SIGCANCEL only while running the
     notification function; there an interval re-fire or a pthread_cancel can
     be delivered to the SIGCANCEL handler, which must therefore already be
     installed.  */
  __pthread_install_sigcancel_handler ();

  /* Block all signals in the helper thread but SIGSETXID.  SIGTIMER stays
     blocked so it can be consumed synchronously with sigwaitinfo between
     firings; it is unblocked only around the notification function.  */
  sigset_t ss;
  __sigfillset (&ss);
  __sigdelset (&ss, SIGSETXID);
  if (__pthread_attr_setsigmask_internal (attr, &ss) < 0)
    return -1;

  struct timer_helper_thread_args_t args = { .evp = evp };
  __pthread_barrier_init (&args.barrier, NULL, 2);

  pthread_t th;
  int r = __pthread_create (&th, attr, timer_helper_thread, &args);
  if (r != 0)
    {
      __set_errno (r);
      return -1;
    }

  struct pthread *pthr = (struct pthread *)th;
  /* SIGEV_THREAD_ID delivers the signal to a specific thread by TID.
     SIGEV_SIGNAL is not combined here because SIGEV_THREAD_ID already
     implies signal delivery; the kernel treats them as orthogonal bits
     and the TID field alone is sufficient to route SIGTIMER correctly.  */
  struct sigevent kevp =
    {
      .sigev_value.sival_ptr = NULL,
      .sigev_signo = SIGTIMER,
      .sigev_notify = SIGEV_THREAD_ID,
      ._sigev_un = { ._tid = pthr->tid },
    };

  /* Use INTERNAL_SYSCALL_CALL so the error code can be captured directly
     without going through errno; errno is set only after the barrier
     wait below, which would otherwise be free to clobber it.  */
  kernel_timer_t ktimerid;
  long int sc = INTERNAL_SYSCALL_CALL (timer_create, clockid, &kevp,
				       &ktimerid);
  if (INTERNAL_SYSCALL_ERROR_P (sc))
    {
      ktimerid = -1;
      /* On timer creation failure we need to signal the helper thread to
	 exit and we cannot use a negative timerid value after the
	 pthread_barrier_wait because we cannot distinguish between a timer
	 creation failure and a request to delete a timer if it happens to
	 arrive quickly (e.g. two timers are created in sequence, where the
	 first succeeds).

	 We re-use the 'exiting' member to signal the failure, it is set only
	 at pthread_create to prevent pthread_kill from sending further
	 signals.  Since the thread should not be user-visible, signals are
	 only sent during timer_delete.  */
      pthr->exiting = true;
    }
  pthr->timerid = ktimerid;
  pthr->timer_overrun = 0;
  /* Signal the thread to continue execution after it copies the arguments
     or exit if the timer can not be created.  */
  __pthread_barrier_wait (&args.barrier);

  if (ktimerid < 0)
    {
      __set_errno (INTERNAL_SYSCALL_ERRNO (sc));
      return -1;
    }

  *timerid = pthread_to_timerid (th);

  return 0;
}

int
___timer_create (clockid_t clock_id, struct sigevent *evp, timer_t *timerid)
{
  clockid_t syscall_clockid = (clock_id == CLOCK_PROCESS_CPUTIME_ID
			       ? PROCESS_CLOCK
			       : clock_id == CLOCK_THREAD_CPUTIME_ID
			       ? THREAD_CLOCK
			       : clock_id);

  switch (evp != NULL ? evp->sigev_notify : SIGEV_SIGNAL)
    {
    case SIGEV_NONE:
    case SIGEV_SIGNAL:
    case SIGEV_THREAD_ID:
      {
	struct sigevent kevp;
	if (evp == NULL)
	  {
	    kevp.sigev_notify = SIGEV_SIGNAL;
	    kevp.sigev_signo = SIGALRM;
	    kevp.sigev_value.sival_ptr = NULL;
	    evp = &kevp;
	  }

	kernel_timer_t ktimerid;
	if (INLINE_SYSCALL_CALL (timer_create, syscall_clockid, evp,
				 &ktimerid) == -1)
	  return -1;

	*timerid = kernel_timer_to_timerid (ktimerid);
      }
      break;
    case SIGEV_THREAD:
      {
	pthread_attr_t attr;
	if (evp->sigev_notify_attributes != NULL)
	  {
	    int r = __pthread_attr_copy (&attr, evp->sigev_notify_attributes);
	    if (r != 0)
	      {
		__set_errno (r);
		return -1;
	      }
	  }
	else
	  __pthread_attr_init (&attr);
	__pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

        int r = timer_create_sigev_thread (syscall_clockid, evp, timerid,
					   &attr);

	__pthread_attr_destroy (&attr);

	return r;
      }
    default:
      __set_errno (EINVAL);
      return -1;
    }

  return 0;
}
versioned_symbol (libc, ___timer_create, timer_create, GLIBC_2_34);
libc_hidden_ver (___timer_create, __timer_create)

#if TIMER_T_WAS_INT_COMPAT
# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_3_3, GLIBC_2_34)
compat_symbol (librt, ___timer_create, timer_create, GLIBC_2_3_3);
# endif

# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_3_3)
timer_t __timer_compat_list[OLD_TIMER_MAX];

int
__timer_create_old (clockid_t clock_id, struct sigevent *evp, int *timerid)
{
  timer_t newp;

  int res = __timer_create (clock_id, evp, &newp);
  if (res == 0)
    {
      int i;
      for (i = 0; i < OLD_TIMER_MAX; ++i)
	if (__timer_compat_list[i] == NULL
	    && ! atomic_compare_and_exchange_bool_acq (&__timer_compat_list[i],
						       newp, NULL))
	  {
	    *timerid = i;
	    break;
	  }

      if (__glibc_unlikely (i == OLD_TIMER_MAX))
	{
	  /* No free slot.  */
	  __timer_delete (newp);
	  __set_errno (EINVAL);
	  res = -1;
	}
    }

  return res;
}
compat_symbol (librt, __timer_create_old, timer_create, GLIBC_2_2);
# endif /* OTHER_SHLIB_COMPAT */

#else /* !TIMER_T_WAS_INT_COMPAT */
# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_34)
compat_symbol (librt, ___timer_create, timer_create, GLIBC_2_2);
# endif
#endif /* !TIMER_T_WAS_INT_COMPAT */
