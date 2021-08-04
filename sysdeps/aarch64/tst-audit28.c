/* Check DT_AUDIT for aarch64 ABI specifics.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#include <array_length.h>
#include <string.h>
#include <support/check.h>
#include <sys/auxv.h>
#include "tst-audit28mod.h"

int
do_test (void)
{
  unsigned long hwcap = getauxval (AT_HWCAP);
  if ((hwcap & HWCAP_SVE) == 0)
    FAIL_UNSUPPORTED ("system does not support SVE");

  {
    svint8_t r = tst_audit28_func_sve_args (sve_args_z0 (), sve_args_z1 (),
					    sve_args_z2 (), sve_args_z3 (),
					    sve_args_z4 (), sve_args_z5 (),
					    sve_args_z6 (), sve_args_z7 ());
    if (!svptest_any (svptrue_b8  (),  svcmpeq_s8  (svptrue_b8 (), r, sve_ret ())))
      FAIL_EXIT1 ("tst_audit28_func_sve_args(): wrong return value");
  }

  return 0;
}

#include <support/test-driver.c>
