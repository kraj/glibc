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
do_test (void)
{
  {
    int r = tst_audit29_func_int (FUNC_INT_ARG0, FUNC_INT_ARG1, FUNC_INT_ARG2,
				  FUNC_INT_ARG3, FUNC_INT_ARG4, FUNC_INT_ARG5,
				  FUNC_INT_ARG6, FUNC_INT_ARG7);
    if (r != FUNC_INT_RET)
      FAIL_EXIT1 ("tst_audit29_func_int returned %d, expected %d", r,
		  FUNC_INT_RET);
  }

  {
    long int r = tst_audit29_func_long (FUNC_LONG_ARG0, FUNC_LONG_ARG1,
					FUNC_LONG_ARG2, FUNC_LONG_ARG3,
					FUNC_LONG_ARG4, FUNC_LONG_ARG5,
					FUNC_LONG_ARG6, FUNC_LONG_ARG7);
    if (r != FUNC_LONG_RET)
      FAIL_EXIT1 ("tst_audit29_func_long returned %ld, expected %ld", r,
		  FUNC_LONG_RET);
  }

  {
    /* Arguments a8 through a11 are passed on the caller stack.  The audit
       module also inspects them through lr_sp in pltexit.  */
    long int r = tst_audit29_func_stack_long (
      FUNC_LONG_ARG0, FUNC_LONG_ARG1, FUNC_LONG_ARG2,
      FUNC_LONG_ARG3, FUNC_LONG_ARG4, FUNC_LONG_ARG5,
      FUNC_LONG_ARG6, FUNC_LONG_ARG7, FUNC_STACK_LONG_ARG8,
      FUNC_STACK_LONG_ARG9, FUNC_STACK_LONG_ARG10, FUNC_STACK_LONG_ARG11);
    if (r != FUNC_STACK_LONG_RET)
      FAIL_EXIT1 ("tst_audit29_func_stack_long returned %ld, expected %ld",
		  r, FUNC_STACK_LONG_RET);
  }

  {
    /* The result is returned in a0 and a1.  This case checks that pltexit does
       not change it.  */
    struct tst_audit29_ret2 r = tst_audit29_func_ret2 ();
    if (r.a0 != FUNC_RET2_A0 || r.a1 != FUNC_RET2_A1)
      FAIL_EXIT1 ("tst_audit29_func_ret2 returned %ld/%ld, expected %ld/%ld",
		  r.a0, r.a1, FUNC_RET2_A0, FUNC_RET2_A1);
  }

  {
    /* pltexit rewrites both return registers.  Receiving the modified values
       here proves that the trampoline reloaded outregs after the callback.  */
    struct tst_audit29_ret2 r = tst_audit29_func_ret2_modified ();
    if (r.a0 != FUNC_RET2_A0_MODIFIED || r.a1 != FUNC_RET2_A1_MODIFIED)
      FAIL_EXIT1 ("tst_audit29_func_ret2_modified returned %ld/%ld, "
		  "expected %ld/%ld", r.a0, r.a1, FUNC_RET2_A0_MODIFIED,
		  FUNC_RET2_A1_MODIFIED);
  }

  {
    /* The return value is larger than two registers and uses a0 as the
       caller-provided stack result buffer.  */
    struct tst_audit29_ret_large r
      = tst_audit29_func_ret_large (FUNC_RET_LARGE_ARG0, FUNC_RET_LARGE_ARG1,
				    FUNC_RET_LARGE_ARG2);
    struct tst_audit29_ret_large expected
      = make_tst_audit29_ret_large (FUNC_RET_LARGE_ARG0, FUNC_RET_LARGE_ARG1,
				    FUNC_RET_LARGE_ARG2);

    TEST_COMPARE_BLOB (&r, sizeof (r), &expected, sizeof (expected));
  }

#ifndef __loongarch_soft_float
  /* The audit module clobbers fa0 through fa7 after each callback.  These
     calls pass all eight floating-point arguments in those registers and also
     return through them, so success requires the trampoline to reload both the
     argument and result registers.  */
  {
    float r = tst_audit29_func_float (FUNC_FLOAT_ARG0, FUNC_FLOAT_ARG1,
				      FUNC_FLOAT_ARG2, FUNC_FLOAT_ARG3,
				      FUNC_FLOAT_ARG4, FUNC_FLOAT_ARG5,
				      FUNC_FLOAT_ARG6, FUNC_FLOAT_ARG7);
    if (r != FUNC_FLOAT_RET)
      FAIL_EXIT1 ("tst_audit29_func_float returned %a, expected %a", r,
		  FUNC_FLOAT_RET);
  }

  {
    /* The result is returned in fa0 and fa1.  This case checks that pltexit
       does not change it.  */
    struct tst_audit29_ret2_float r = tst_audit29_func_ret2_float ();
    if (r.a0 != FUNC_RET2_FP_A0 || r.a1 != FUNC_RET2_FP_A1)
      FAIL_EXIT1 ("tst_audit29_func_ret2_float returned %a/%a, expected %a/%a",
		  r.a0, r.a1, FUNC_RET2_FP_A0, FUNC_RET2_FP_A1);
  }

  {
    /* This case checks that pltexit rewrites the return result in fa0 and
       fa1.  */
    struct tst_audit29_ret2_float r = tst_audit29_func_ret2_float_modified ();
    if (r.a0 != FUNC_RET2_FP_A0_MODIFIED || r.a1 != FUNC_RET2_FP_A1_MODIFIED)
      FAIL_EXIT1 ("tst_audit29_func_ret2_float_modified returned %a/%a, "
		  "expected %a/%a", r.a0, r.a1, FUNC_RET2_FP_A0_MODIFIED,
		  FUNC_RET2_FP_A1_MODIFIED);
  }
#if __loongarch_frlen == 64
  {
    double r = tst_audit29_func_double (FUNC_DOUBLE_ARG0, FUNC_DOUBLE_ARG1,
					FUNC_DOUBLE_ARG2, FUNC_DOUBLE_ARG3,
					FUNC_DOUBLE_ARG4, FUNC_DOUBLE_ARG5,
					FUNC_DOUBLE_ARG6, FUNC_DOUBLE_ARG7);
    if (r != FUNC_DOUBLE_RET)
      FAIL_EXIT1 ("tst_audit29_func_double returned %a, expected %a", r,
		  FUNC_DOUBLE_RET);
  }

  {
    struct tst_audit29_ret2_double r = tst_audit29_func_ret2_double ();
    if (r.a0 != FUNC_RET2_FP_A0 || r.a1 != FUNC_RET2_FP_A1)
      FAIL_EXIT1 ("tst_audit29_func_ret2_double returned %a/%a, "
		  "expected %a/%a", r.a0, r.a1, FUNC_RET2_FP_A0,
		  FUNC_RET2_FP_A1);
  }

  {
    struct tst_audit29_ret2_double r
      = tst_audit29_func_ret2_double_modified ();
    if (r.a0 != FUNC_RET2_FP_A0_MODIFIED || r.a1 != FUNC_RET2_FP_A1_MODIFIED)
      FAIL_EXIT1 ("tst_audit29_func_ret2_double_modified returned %a/%a, "
		  "expected %a/%a", r.a0, r.a1, FUNC_RET2_FP_A0_MODIFIED,
		  FUNC_RET2_FP_A1_MODIFIED);
  }
#endif
#endif

  return 0;
}

#include <support/test-driver.c>
