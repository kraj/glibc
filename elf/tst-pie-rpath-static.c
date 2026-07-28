/* Check if RPATH/RUNPATH is allowed for static-pie.
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

#include <support/check.h>
#include <support/xdlfcn.h>

/* The module is placed in the directory named by the RUNPATH of this
   program (tst-pie-rpath-static.root/tst-pie-rpath-static.script), so
   dlopen can only find it if the RUNPATH is honoured.  */
#define LIBNAME "tst-pie-rpath-mod.so"

static int
do_test (void)
{
  void *h = xdlopen (LIBNAME, RTLD_NOW);
  int (*foo)(void) = xdlsym (h, "foo");
  TEST_COMPARE (foo (), 42);
  xdlclose (h);

  return 0;
}

#include <support/test-driver.c>
