/* Check LD_AUDIT for LoongArch ABI specifics.
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
#include "tst-audit29mod.h"

int
tst_audit29_func_int (int a0, int a1, int a2, int a3, int a4, int a5, int a6,
		      int a7)
{
  if (a0 != FUNC_INT_ARG0)
    FAIL_EXIT1 ("a0: %d != %d", a0, FUNC_INT_ARG0);
  if (a1 != FUNC_INT_ARG1)
    FAIL_EXIT1 ("a1: %d != %d", a1, FUNC_INT_ARG1);
  if (a2 != FUNC_INT_ARG2)
    FAIL_EXIT1 ("a2: %d != %d", a2, FUNC_INT_ARG2);
  if (a3 != FUNC_INT_ARG3)
    FAIL_EXIT1 ("a3: %d != %d", a3, FUNC_INT_ARG3);
  if (a4 != FUNC_INT_ARG4)
    FAIL_EXIT1 ("a4: %d != %d", a4, FUNC_INT_ARG4);
  if (a5 != FUNC_INT_ARG5)
    FAIL_EXIT1 ("a5: %d != %d", a5, FUNC_INT_ARG5);
  if (a6 != FUNC_INT_ARG6)
    FAIL_EXIT1 ("a6: %d != %d", a6, FUNC_INT_ARG6);
  if (a7 != FUNC_INT_ARG7)
    FAIL_EXIT1 ("a7: %d != %d", a7, FUNC_INT_ARG7);

  return FUNC_INT_RET;
}

long int
tst_audit29_func_long (long int a0, long int a1, long int a2, long int a3,
		       long int a4, long int a5, long int a6, long int a7)
{
  if (a0 != FUNC_LONG_ARG0)
    FAIL_EXIT1 ("a0: %ld != %ld", a0, FUNC_LONG_ARG0);
  if (a1 != FUNC_LONG_ARG1)
    FAIL_EXIT1 ("a1: %ld != %ld", a1, FUNC_LONG_ARG1);
  if (a2 != FUNC_LONG_ARG2)
    FAIL_EXIT1 ("a2: %ld != %ld", a2, FUNC_LONG_ARG2);
  if (a3 != FUNC_LONG_ARG3)
    FAIL_EXIT1 ("a3: %ld != %ld", a3, FUNC_LONG_ARG3);
  if (a4 != FUNC_LONG_ARG4)
    FAIL_EXIT1 ("a4: %ld != %ld", a4, FUNC_LONG_ARG4);
  if (a5 != FUNC_LONG_ARG5)
    FAIL_EXIT1 ("a5: %ld != %ld", a5, FUNC_LONG_ARG5);
  if (a6 != FUNC_LONG_ARG6)
    FAIL_EXIT1 ("a6: %ld != %ld", a6, FUNC_LONG_ARG6);
  if (a7 != FUNC_LONG_ARG7)
    FAIL_EXIT1 ("a7: %ld != %ld", a7, FUNC_LONG_ARG7);

  return FUNC_LONG_RET;
}

long int
tst_audit29_func_stack_long (long int a0, long int a1, long int a2,
			     long int a3, long int a4, long int a5,
			     long int a6, long int a7, long int a8,
			     long int a9, long int a10, long int a11)
{
  const long int actual[] = {
    a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11
  };
  const long int expected[] = {
    FUNC_LONG_ARG0, FUNC_LONG_ARG1, FUNC_LONG_ARG2,
    FUNC_LONG_ARG3, FUNC_LONG_ARG4, FUNC_LONG_ARG5,
    FUNC_LONG_ARG6, FUNC_LONG_ARG7, FUNC_STACK_LONG_ARG8,
    FUNC_STACK_LONG_ARG9, FUNC_STACK_LONG_ARG10, FUNC_STACK_LONG_ARG11
  };

  for (size_t i = 0; i < sizeof (actual) / sizeof (actual[0]); ++i)
    if (actual[i] != expected[i])
      FAIL_EXIT1 ("a%zu: %ld != %ld", i, actual[i], expected[i]);

  return FUNC_STACK_LONG_RET;
}

struct tst_audit29_ret2
tst_audit29_func_ret2 (void)
{
  struct tst_audit29_ret2 ret = {FUNC_RET2_A0, FUNC_RET2_A1};
  return ret;
}

struct tst_audit29_ret2
tst_audit29_func_ret2_modified (void)
{
  struct tst_audit29_ret2 ret = {FUNC_RET2_A0, FUNC_RET2_A1};
  return ret;
}

struct tst_audit29_ret_large
tst_audit29_func_ret_large (char a, short b, long int c)
{
  if (a != FUNC_RET_LARGE_ARG0)
    FAIL_EXIT1 ("a: %#x != %#x", a, FUNC_RET_LARGE_ARG0);
  if (b != FUNC_RET_LARGE_ARG1)
    FAIL_EXIT1 ("b: %#x != %#x", b, FUNC_RET_LARGE_ARG1);
  if (c != FUNC_RET_LARGE_ARG2)
    FAIL_EXIT1 ("c: %#lx != %#lx", c, FUNC_RET_LARGE_ARG2);

  return make_tst_audit29_ret_large (a, b, c);
}

#ifndef __loongarch_soft_float
float
tst_audit29_func_float (float a0, float a1, float a2, float a3, float a4,
			float a5, float a6, float a7)
{
  if (a0 != FUNC_FLOAT_ARG0)
    FAIL_EXIT1 ("a0: %a != %a", a0, FUNC_FLOAT_ARG0);
  if (a1 != FUNC_FLOAT_ARG1)
    FAIL_EXIT1 ("a1: %a != %a", a1, FUNC_FLOAT_ARG1);
  if (a2 != FUNC_FLOAT_ARG2)
    FAIL_EXIT1 ("a2: %a != %a", a2, FUNC_FLOAT_ARG2);
  if (a3 != FUNC_FLOAT_ARG3)
    FAIL_EXIT1 ("a3: %a != %a", a3, FUNC_FLOAT_ARG3);
  if (a4 != FUNC_FLOAT_ARG4)
    FAIL_EXIT1 ("a4: %a != %a", a4, FUNC_FLOAT_ARG4);
  if (a5 != FUNC_FLOAT_ARG5)
    FAIL_EXIT1 ("a5: %a != %a", a5, FUNC_FLOAT_ARG5);
  if (a6 != FUNC_FLOAT_ARG6)
    FAIL_EXIT1 ("a6: %a != %a", a6, FUNC_FLOAT_ARG6);
  if (a7 != FUNC_FLOAT_ARG7)
    FAIL_EXIT1 ("a7: %a != %a", a7, FUNC_FLOAT_ARG7);

  return FUNC_FLOAT_RET;
}

struct tst_audit29_ret2_float
tst_audit29_func_ret2_float (void)
{
  struct tst_audit29_ret2_float ret = {FUNC_RET2_FP_A0, FUNC_RET2_FP_A1};
  return ret;
}

struct tst_audit29_ret2_float
tst_audit29_func_ret2_float_modified (void)
{
  struct tst_audit29_ret2_float ret = {FUNC_RET2_FP_A0, FUNC_RET2_FP_A1};
  return ret;
}

#if __loongarch_frlen == 64
double
tst_audit29_func_double (double a0, double a1, double a2, double a3, double a4,
			 double a5, double a6, double a7)
{
  if (a0 != FUNC_DOUBLE_ARG0)
    FAIL_EXIT1 ("a0: %a != %a", a0, FUNC_DOUBLE_ARG0);
  if (a1 != FUNC_DOUBLE_ARG1)
    FAIL_EXIT1 ("a1: %a != %a", a1, FUNC_DOUBLE_ARG1);
  if (a2 != FUNC_DOUBLE_ARG2)
    FAIL_EXIT1 ("a2: %a != %a", a2, FUNC_DOUBLE_ARG2);
  if (a3 != FUNC_DOUBLE_ARG3)
    FAIL_EXIT1 ("a3: %a != %a", a3, FUNC_DOUBLE_ARG3);
  if (a4 != FUNC_DOUBLE_ARG4)
    FAIL_EXIT1 ("a4: %a != %a", a4, FUNC_DOUBLE_ARG4);
  if (a5 != FUNC_DOUBLE_ARG5)
    FAIL_EXIT1 ("a5: %a != %a", a5, FUNC_DOUBLE_ARG5);
  if (a6 != FUNC_DOUBLE_ARG6)
    FAIL_EXIT1 ("a6: %a != %a", a6, FUNC_DOUBLE_ARG6);
  if (a7 != FUNC_DOUBLE_ARG7)
    FAIL_EXIT1 ("a7: %a != %a", a7, FUNC_DOUBLE_ARG7);

  return FUNC_DOUBLE_RET;
}

struct tst_audit29_ret2_double
tst_audit29_func_ret2_double (void)
{
  struct tst_audit29_ret2_double ret = {FUNC_RET2_FP_A0, FUNC_RET2_FP_A1};
  return ret;
}

struct tst_audit29_ret2_double
tst_audit29_func_ret2_double_modified (void)
{
  struct tst_audit29_ret2_double ret = {FUNC_RET2_FP_A0, FUNC_RET2_FP_A1};
  return ret;
}
#endif
#endif
