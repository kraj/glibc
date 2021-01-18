/* _Fork implementation.  Linux version.
   Copyright (C) 2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <arch-fork.h>
#include <nptl/pthreadP.h>

/* Pointer to the fork generation counter in the thread library.  */
extern unsigned long int *__fork_generation_pointer attribute_hidden;

pid_t
_Fork (void)
{
  pid_t pid = arch_fork (&THREAD_SELF->tid);
  if (pid == 0)
    {
      struct pthread *self = THREAD_SELF;

      /* See __pthread_once.  */
      if (__fork_generation_pointer != NULL)
	*__fork_generation_pointer += __PTHREAD_ONCE_FORK_GEN_INCR;

      /* Initialize the robust mutex list setting in the kernel which has
	 been reset during the fork.  We do not check for errors because if
	 it fails here, it must have failed at process startup as well and
	 nobody could have used robust mutexes.
	 Before we do that, we have to clear the list of robust mutexes
	 because we do not inherit ownership of mutexes from the parent.
	 We do not have to set self->robust_head.futex_offset since we do
	 inherit the correct value from the parent.  We do not need to clear
	 the pending operation because it must have been zero when fork was
	 called.  */
#if __PTHREAD_MUTEX_HAVE_PREV
      self->robust_prev = &self->robust_head;
#endif
      self->robust_head.list = &self->robust_head;
#ifdef SHARED
      if (__builtin_expect (__libc_pthread_functions_init, 0))
	PTHFCT_CALL (ptr_set_robust, (self));
#else
      extern __typeof (__nptl_set_robust) __nptl_set_robust
	__attribute__((weak));
      if (__builtin_expect (__nptl_set_robust != NULL, 0))
	__nptl_set_robust (self);
#endif
    }
  return pid;
}
libc_hidden_def (_Fork)
