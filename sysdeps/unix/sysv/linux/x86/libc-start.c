/* Copyright (C) 2017 Free Software Foundation, Inc.
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

#ifdef ENABLE_CET
# include <link.h>
# define ARCH_INIT_CPU_FEATURES() \
  {								\
    init_cpu_features (&_dl_x86_cpu_features);			\
    _dl_setup_cet (_dl_phdr, _dl_phnum, 0);			\
  }
#endif

#include <sysdeps/x86/libc-start.c>
