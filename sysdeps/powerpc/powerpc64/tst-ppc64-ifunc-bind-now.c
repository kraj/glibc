/* Regression test for BZ #34503: powerpc IFUNC resolver crash with BIND_NOW.
   Copyright (C) 2026 Free Software Foundation, Inc.
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

/* Verify that calling an IFUNC function (modf from libm) via a function
   pointer exported from a shared library that was linked with -z,now and
   has no DT_NEEDED on libm does not crash.

   Without the fix, INIT_ARCH() in the modf IFUNC resolver dereferences
   _rtld_global_ro directly; if that pointer has not been relocated yet
   in the module's GOT context, the dereference faults.  With the fix,
   the __GLRO() wrapper returns 0 when _rtld_global_ro is NULL and the
   resolver falls back to the default implementation.  */

#include <math.h>
#include <stdio.h>
#include <support/check.h>

/* Exported by tst-ppc64-ifunc-bind-now-mod.so, which is built with
   -z,now and has no DT_NEEDED on libm.  */
extern double (*volatile ifunc_ptr) (double, double *);

static int
do_test (void)
{
  double ip;
  double frac = ifunc_ptr (2.5, &ip);
  printf ("modf(2.5) = %g + %g (expect 0.5 + 2)\n", frac, ip);
  TEST_VERIFY (frac == 0.5);
  TEST_VERIFY (ip == 2.0);
  return 0;
}

#include <support/test-driver.c>
