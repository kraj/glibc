/* Initialization code run first thing by the ELF startup code.  Linux/ARM.
   Copyright (C) 2015-2017 Free Software Foundation, Inc.

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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef SHARED
# include <dl-vdso.h>
# include <libc-vdso.h>

int (*VDSO_SYMBOL(gettimeofday)) (struct timeval *, void *) attribute_hidden;
int (*VDSO_SYMBOL(clock_gettime)) (clockid_t, struct timespec *);
long (*VDSO_SYMBOL(clock_gettime64)) (clockid_t, struct __timespec64 *);

int __y2038_linux_support;

int __y2038_kernel_support (void)
{
  return __y2038_linux_support;
}

static inline void
_libc_vdso_platform_setup (void)
{
  PREPARE_VERSION_KNOWN (linux26, LINUX_2_6);

  void *p = _dl_vdso_vsym ("__vdso_gettimeofday", &linux26);
  PTR_MANGLE (p);
  VDSO_SYMBOL (gettimeofday) = p;

  p = _dl_vdso_vsym ("__vdso_clock_gettime", &linux26);
  PTR_MANGLE (p);
  VDSO_SYMBOL (clock_gettime) = p;

  /* (aaribaud) TODO: map to version where clock_gettime64 officially appears */
  p = _dl_vdso_vsym ("__vdso_clock_gettime64", NULL);
  PTR_MANGLE (p);
  VDSO_SYMBOL (clock_gettime64) = p;

  __y2038_linux_support = (p != NULL) ? 1 : 0;
}

# define VDSO_SETUP _libc_vdso_platform_setup
#endif

#include <csu/init-first.c>
