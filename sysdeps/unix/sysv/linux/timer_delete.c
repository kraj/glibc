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

#include <unistd.h>
#include <time.h>
#include "kernel-posix-timers.h"
#include <pthreadP.h>
#include <shlib-compat.h>

int
___timer_delete (timer_t timerid)
{
  if (timer_is_sigev_thread (timerid))
    {
      struct pthread *th = timerid_to_pthread (timerid);
      kernel_timer_t ktimerid = timerid_to_kernel_timer (timerid);

      /* Delete the kernel timer first so no new events are generated
	 after this function returns.  */
      int ret = INLINE_SYSCALL_CALL (timer_delete, ktimerid);
      if (ret != 0)
	return ret;

      /* Signal the helper thread to exit its sigwaitinfo loop.  */
      timerid_signal_delete (&th->timerid);
      /* The helper threads leaves SIGTIMER/SIGCANCEL unblocked while running
	 the notification function, and a tgkill would  arrive as SI_TKILL
	 and could be mistaken for a cancellation by the SIGCANCEL handler.
	  With SI_QUEUE the handler ignores it (only SI_TKILL cancels), while
	 a helper blocked in sigwaitinfo still wakes up, observes the MSB set
	 above (si_code != SI_TIMER), and exits.  */
      siginfo_t info = { 0 };
      info.si_signo = SIGTIMER;
      info.si_code = SI_QUEUE;
      INTERNAL_SYSCALL_CALL (rt_tgsigqueueinfo, __getpid (), th->tid,
			     SIGTIMER, &info);
      return 0;
    }
  return INLINE_SYSCALL_CALL (timer_delete, timerid);
}
versioned_symbol (libc, ___timer_delete, timer_delete, GLIBC_2_34);
libc_hidden_ver (___timer_delete, __timer_delete)

#if TIMER_T_WAS_INT_COMPAT
# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_3_3, GLIBC_2_34)
compat_symbol (librt, ___timer_delete, timer_delete, GLIBC_2_3_3);
#endif

# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_3_3)
int
__timer_delete_old (int timerid)
{
  int res = __timer_delete (__timer_compat_list[timerid]);

  if (res == 0)
    /* Successful timer deletion, now free the index.  We only need to
       store a word and that better be atomic.  */
    __timer_compat_list[timerid] = NULL;

  return res;
}
compat_symbol (librt, __timer_delete_old, timer_delete, GLIBC_2_2);
# endif /* OTHER_SHLIB_COMPAT */

#else /* !TIMER_T_WAS_INT_COMPAT */
/* The transition from int to timer_t did not change ABI because the
   type sizes are the same.  */
# if OTHER_SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_34)
compat_symbol (librt, ___timer_delete, timer_delete, GLIBC_2_2);
# endif
#endif /* !TIMER_T_WAS_INT_COMPAT */
