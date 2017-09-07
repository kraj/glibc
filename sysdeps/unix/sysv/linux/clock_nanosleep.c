/* Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

#include <time.h>
#include <errno.h>

#include <sysdep-cancel.h>
#include "kernel-posix-cpu-timers.h"

/* We can simply use the syscall.  The CPU clocks are not supported
   with this function.  */
int
__clock_nanosleep (clockid_t clock_id, int flags, const struct timespec *req,
		   struct timespec *rem)
{
  INTERNAL_SYSCALL_DECL (err);
  int r;

  if (clock_id == CLOCK_THREAD_CPUTIME_ID)
    return EINVAL;
  if (clock_id == CLOCK_PROCESS_CPUTIME_ID)
    clock_id = MAKE_PROCESS_CPUCLOCK (0, CPUCLOCK_SCHED);

  if (SINGLE_THREAD_P)
    r = INTERNAL_SYSCALL (clock_nanosleep, err, 4, clock_id, flags, req, rem);
  else
    {
      int oldstate = LIBC_CANCEL_ASYNC ();

      r = INTERNAL_SYSCALL (clock_nanosleep, err, 4, clock_id, flags, req,
			    rem);

      LIBC_CANCEL_RESET (oldstate);
    }

  return (INTERNAL_SYSCALL_ERROR_P (r, err)
	  ? INTERNAL_SYSCALL_ERRNO (r, err) : 0);
}
weak_alias (__clock_nanosleep, clock_nanosleep)

/* 64-bit time version */

extern int __y2038_linux_support;

int
__clock_nanosleep64 (clockid_t clock_id, int flags,
		   const struct __timespec64 *req,
                   struct __timespec64 *rem)
{
  INTERNAL_SYSCALL_DECL (err);
  int r;
  struct timespec req32, rem32;
/* Check that we are built with a 64-bit-time kernel */
#ifdef __NR_clock_nanosleep64
  struct __timespec64 req64;
#endif

  if (clock_id == CLOCK_THREAD_CPUTIME_ID)
    return EINVAL;
  if (clock_id == CLOCK_PROCESS_CPUTIME_ID)
    clock_id = MAKE_PROCESS_CPUCLOCK (0, CPUCLOCK_SCHED);

  if (SINGLE_THREAD_P)
    {
/* Check that we are built with a 64-bit-time kernel */
#ifdef __NR_clock_nanosleep64
      if (__y2038_linux_support)
        {
          req64.tv_sec = req->tv_sec;
          req64.tv_nsec = req->tv_nsec;
          req64.tv_pad = 0;
          r = INTERNAL_SYSCALL (clock_nanosleep64, err, 4, clock_id, flags,
                                &req64, rem);
        }
      else
        {
          __set_errno(ENOSYS);
          r = -1;
        }
      if (r == -1 && errno == ENOSYS)
#endif
        {
          if (! timespec64_to_timespec(req, &req32))
            {
              __set_errno(ENOSYS);
              r = -1;
            }
          else
            {
              r = INTERNAL_SYSCALL (clock_nanosleep, err, 4, clock_id, flags,
                                    &req32, &rem32);
              if (r == -1 && errno == EINTR && rem != NULL && flags != TIMER_ABSTIME)
                timespec_to_timespec64(&rem32, rem);
            }
        }
    }
  else
    {
      int oldstate = LIBC_CANCEL_ASYNC ();

/* Check that we are built with a 64-bit-time kernel */
#ifdef __NR_clock_nanosleep64
      if (__y2038_linux_support)
        {
          req64.tv_sec = req->tv_sec;
          req64.tv_nsec = req->tv_nsec;
          req64.tv_pad = 0;
          r = INTERNAL_SYSCALL (clock_nanosleep64, err, 4, clock_id, flags,
                                &req64, rem);
        }
      else
        {
          __set_errno(ENOSYS);
          r = -1;
        }
      if (r == -1 && errno == ENOSYS)
#endif
        {
          if (! timespec64_to_timespec(req, &req32))
            {
              __set_errno(ENOSYS);
              r = -1;
            }
          else
            {
              r = INTERNAL_SYSCALL (clock_nanosleep, err, 4, clock_id, flags,
                                    &req32, &rem32);
              if (r == -1 && errno == EINTR && rem != NULL && flags != TIMER_ABSTIME)
                timespec_to_timespec64(&rem32, rem);
            }
        }
      
      LIBC_CANCEL_RESET (oldstate);
    }

  if (INTERNAL_SYSCALL_ERROR_P (r, err))
    {
      return INTERNAL_SYSCALL_ERRNO (r, err);
    }
  else
    {
      return 0;
    }
}
