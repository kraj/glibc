/* Copyright (C) 2003-2016 Free Software Foundation, Inc.
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
#include <sysdep.h>
#include <time.h>
#include <kernel_timespec.h>

#include "kernel-posix-cpu-timers.h"


/* The REALTIME clock is definitely supported in the kernel.  */
#define SYSDEP_SETTIME64 \
  case CLOCK_REALTIME:							      \
    {                                                                         \
      struct kernel_timespec64 ts64;                                          \
      ts64.tv_sec = tp->tv_sec;                                               \
      ts64.tv_nsec = tp->tv_nsec;                                             \
      retval = INLINE_SYSCALL (clock_settime64, 2, clock_id, &ts64);          \
      if (retval == ENOSYS) {                                                 \
        if (tp->tv_sec > LONG_MAX) {                                          \
          retval = EOVERFLOW;                                                 \
        }                                                                     \
        else                                                                  \
        {                                                                     \
          struct kernel_timespec32 ts32;                                      \
          ts32.tv_sec = tp->tv_sec;                                           \
          ts32.tv_nsec = tp->tv_nsec;                                         \
          retval = INLINE_SYSCALL (clock_settime, 2, clock_id, &ts32);        \
        }                                                                     \
      }                                                                       \
    }                                                                         \
    break

#define SYSDEP_SETTIME \
  case CLOCK_REALTIME:							      \
    retval = INLINE_SYSCALL (clock_settime, 2, clock_id, tp);		      \
    break

/* We handled the REALTIME clock here.  */
#define HANDLED_REALTIME	1

#define HANDLED_CPUTIME 1
#define SYSDEP_SETTIME64_CPU \
  retval = INLINE_SYSCALL (clock_settime64, 2, clock_id, tp)

#define SYSDEP_SETTIME_CPU \
  retval = INLINE_SYSCALL (clock_settime, 2, clock_id, tp)

#include <sysdeps/unix/clock_settime.c>
