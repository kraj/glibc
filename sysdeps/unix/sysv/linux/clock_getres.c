/* clock_getres -- Get the resolution of a POSIX clockid_t.  Linux version.
   Copyright (C) 2003-2017 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <errno.h>
#include <time.h>
#include "kernel-posix-cpu-timers.h"

#ifdef HAVE_CLOCK_GETRES_VSYSCALL
# define HAVE_VSYSCALL
#endif
#include <sysdep-vdso.h>

#define SYSCALL_GETRES \
  retval = INLINE_VSYSCALL (clock_getres, 2, clock_id, res); \
  break

/* The REALTIME and MONOTONIC clock are definitely supported in the
   kernel.  */
#define SYSDEP_GETRES							      \
  SYSDEP_GETRES_CPUTIME							      \
  case CLOCK_REALTIME:							      \
  case CLOCK_MONOTONIC:							      \
  case CLOCK_MONOTONIC_RAW:						      \
  case CLOCK_REALTIME_COARSE:						      \
  case CLOCK_MONOTONIC_COARSE:						      \
    SYSCALL_GETRES

/* We handled the REALTIME clock here.  */
#define HANDLED_REALTIME	1
#define HANDLED_CPUTIME		1

#define SYSDEP_GETRES_CPU SYSCALL_GETRES
#define SYSDEP_GETRES_CPUTIME	/* Default catches them too.  */

/* The 64-bit version */

extern int __y2038_linux_support;

#define SYSCALL_GETRES64 \
  if (__y2038_linux_support)						      \
  {									      \
    retval = INLINE_VSYSCALL (clock_getres64, 2, clock_id, res);  	      \
  }									      \
  else									      \
  {									      \
    retval = INLINE_VSYSCALL (clock_getres, 2, clock_id, &ts32);	      \
    if (retval==0)							      \
    {									      \
      res->tv_sec = ts32.tv_sec;		                	      \
      res->tv_nsec = ts32.tv_nsec;					      \
      res->tv_pad = 0;				                	      \
    }									      \
  }									      \
  break

/* The REALTIME and MONOTONIC clock are definitely supported in the
   kernel.  */
#define SYSDEP_GETRES64							      \
  SYSDEP_GETRES_CPUTIME64							      \
  case CLOCK_REALTIME:							      \
  case CLOCK_MONOTONIC:							      \
  case CLOCK_MONOTONIC_RAW:						      \
  case CLOCK_REALTIME_COARSE:						      \
  case CLOCK_MONOTONIC_COARSE:						      \
    SYSCALL_GETRES64

/* We handled the REALTIME clock here.  */
#define HANDLED_REALTIME64	1
#define HANDLED_CPUTIME64	1

#define SYSDEP_GETRES_CPU64 SYSCALL_GETRES64
#define SYSDEP_GETRES_CPUTIME64 \
  struct timespec ts32;
 
#include <sysdeps/posix/clock_getres.c>
