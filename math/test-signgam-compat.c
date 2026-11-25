/* Test that lgammaf@GLIBC_2.23 does not set signgam (BZ 35053).
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

#include <shlib-compat.h>
#include <support/check.h>

float lgammaf (float);
compat_symbol_reference (libm, lgammaf, lgammaf, GLIBC_2_23);

int signgam = 123;

static void
check (float x, float expected)
{
  volatile float vx = x;
  volatile float y = lgammaf (vx);
  TEST_VERIFY (y == expected);
  TEST_COMPARE (signgam, 123);
}

static int
do_test (void)
{
  check (0.5f, 0x1.250d04p-1f);
  check (-0.5f, 0x1.43f89ap+0f);
  return 0;
}

#include <support/test-driver.c>
