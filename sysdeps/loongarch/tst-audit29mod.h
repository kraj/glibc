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

#ifndef _TST_AUDIT29MOD_H
#define _TST_AUDIT29MOD_H 1

#include <array_length.h>
#include <float.h>
#include <limits.h>

#define FUNC_INT_ARG0 INT_MIN
#define FUNC_INT_ARG1 INT_MAX
#define FUNC_INT_ARG2 -1
#define FUNC_INT_ARG3 0
#define FUNC_INT_ARG4 1
#define FUNC_INT_ARG5 2
#define FUNC_INT_ARG6 3
#define FUNC_INT_ARG7 4
#define FUNC_INT_RET  5

int
tst_audit29_func_int (int a0, int a1, int a2, int a3, int a4, int a5, int a6,
		      int a7);

#define FUNC_LONG_ARG0 LONG_MIN
#define FUNC_LONG_ARG1 LONG_MAX
#define FUNC_LONG_ARG2 -1L
#define FUNC_LONG_ARG3 0L
#define FUNC_LONG_ARG4 1L
#define FUNC_LONG_ARG5 2L
#define FUNC_LONG_ARG6 3L
#define FUNC_LONG_ARG7 4L
#define FUNC_LONG_RET  5L

#define FUNC_STACK_LONG_ARG8 1008L
#define FUNC_STACK_LONG_ARG9 1009L
#define FUNC_STACK_LONG_ARG10 1010L
#define FUNC_STACK_LONG_ARG11 1011L
#define FUNC_STACK_LONG_RET 1012L

long int
tst_audit29_func_long (long int a0, long int a1, long int a2, long int a3,
		       long int a4, long int a5, long int a6, long int a7);

long int
tst_audit29_func_stack_long (long int a0, long int a1, long int a2,
			     long int a3, long int a4, long int a5,
			     long int a6, long int a7, long int a8,
			     long int a9, long int a10, long int a11);

struct tst_audit29_ret2
{
  long int a0;
  long int a1;
};

#define FUNC_RET2_A0 LONG_MIN
#define FUNC_RET2_A1 LONG_MAX
#define FUNC_RET2_A0_MODIFIED 0x12345678L
#define FUNC_RET2_A1_MODIFIED 0x9abcdef0L

struct tst_audit29_ret2 tst_audit29_func_ret2 (void);
struct tst_audit29_ret2 tst_audit29_func_ret2_modified (void);

struct tst_audit29_ret_large
{
  char a[16];
  short b[8];
  long int c[4];
};

static inline struct tst_audit29_ret_large
make_tst_audit29_ret_large (char a, short b, long int c)
{
  struct tst_audit29_ret_large r;

  for (int i = 0; i < array_length (r.a); ++i)
    r.a[i] = a;
  for (int i = 0; i < array_length (r.b); ++i)
    r.b[i] = b;
  for (int i = 0; i < array_length (r.c); ++i)
    r.c[i] = c;
  return r;
}

#define FUNC_RET_LARGE_ARG0 0x12
#define FUNC_RET_LARGE_ARG1 0x1234
#define FUNC_RET_LARGE_ARG2 0x12345678L

struct tst_audit29_ret_large
tst_audit29_func_ret_large (char a, short b, long int c);

#ifndef __loongarch_soft_float

#define FUNC_FLOAT_ARG0 FLT_MIN
#define FUNC_FLOAT_ARG1 FLT_MAX
#define FUNC_FLOAT_ARG2 FLT_EPSILON
#define FUNC_FLOAT_ARG3 FLT_TRUE_MIN
#define FUNC_FLOAT_ARG4 0.0f
#define FUNC_FLOAT_ARG5 1.0f
#define FUNC_FLOAT_ARG6 2.0f
#define FUNC_FLOAT_ARG7 3.0f
#define FUNC_FLOAT_RET  4.0f

#define FUNC_RET2_FP_A0 1.25
#define FUNC_RET2_FP_A1 -2.5
#define FUNC_RET2_FP_A0_MODIFIED 3.5
#define FUNC_RET2_FP_A1_MODIFIED -6.75

float
tst_audit29_func_float (float a0, float a1, float a2, float a3, float a4,
			float a5, float a6, float a7);

struct tst_audit29_ret2_float
{
  float a0;
  float a1;
};

struct tst_audit29_ret2_float tst_audit29_func_ret2_float (void);
struct tst_audit29_ret2_float tst_audit29_func_ret2_float_modified (void);

#if __loongarch_frlen == 64
#define FUNC_DOUBLE_ARG0 DBL_MIN
#define FUNC_DOUBLE_ARG1 DBL_MAX
#define FUNC_DOUBLE_ARG2 DBL_EPSILON
#define FUNC_DOUBLE_ARG3 DBL_TRUE_MIN
#define FUNC_DOUBLE_ARG4 0.0
#define FUNC_DOUBLE_ARG5 1.0
#define FUNC_DOUBLE_ARG6 2.0
#define FUNC_DOUBLE_ARG7 3.0
#define FUNC_DOUBLE_RET  0x1.fffffe0000001p+127

struct tst_audit29_ret2_double
{
  double a0;
  double a1;
};

struct tst_audit29_ret2_double tst_audit29_func_ret2_double (void);
struct tst_audit29_ret2_double tst_audit29_func_ret2_double_modified (void);
double
tst_audit29_func_double (double a0, double a1, double a2, double a3, double a4,
			 double a5, double a6, double a7);
#endif
#endif
#endif
