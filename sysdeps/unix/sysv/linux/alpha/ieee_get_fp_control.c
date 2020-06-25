/* Copyright (C) 2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#include <fenv_libc.h>
#include <sysdep.h>
#include "kernel_sysinfo.h"

unsigned long int
__ieee_get_fp_control (void)
{
  unsigned long int env;
  int r = inline_syscall (__NR_osf_getsysinfo, GSI_IEEE_FP_CONTROL, &env, 0,
			  0, 0);
  if (r != 0)
    return r;
  return env;
}
libc_hidden_def (__ieee_get_fp_control)
weak_alias (__ieee_get_fp_control, ieee_get_fp_control)
