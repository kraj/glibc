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

#include <assert.h>
#include <link.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/asm.h>
#include "tst-audit29mod.h"

#define TEST_NAME  "tst-audit29"

#define AUDIT29_COOKIE 0

#ifndef __loongarch_soft_float
static float
fpreg_float (const La_loongarch_vector *v)
{
  float value;
  memcpy (&value, v, sizeof (value));
  return value;
}

static void
set_fpreg_float (La_loongarch_vector *v, float value)
{
  memcpy (v, &value, sizeof (value));
}

#if __loongarch_frlen == 64
static double
fpreg_double (const La_loongarch_vector *v)
{
  return v->fpreg[0];
}

static void
set_fpreg_double (La_loongarch_vector *v, double value)
{
  v->fpreg[0] = value;
}
#endif

static void
clobber_fp_regs (void)
{
#define STRINGXP(X) __STRING (X)
  const double clobber = -1.0;
  asm volatile (STRINGXP (FREG_L) " $f0, %0\n\t"
		STRINGXP (FREG_L) " $f1, %0\n\t"
		STRINGXP (FREG_L) " $f2, %0\n\t"
		STRINGXP (FREG_L) " $f3, %0\n\t"
		STRINGXP (FREG_L) " $f4, %0\n\t"
		STRINGXP (FREG_L) " $f5, %0\n\t"
		STRINGXP (FREG_L) " $f6, %0\n\t"
		STRINGXP (FREG_L) " $f7, %0"
		: : "m" (clobber)
		: "$f0", "$f1", "$f2", "$f3", "$f4", "$f5", "$f6", "$f7");
#undef STRINGXP
}
#endif

static void
check_stack_long_args (const La_loongarch_regs *regs)
{
  const long int *stack_args = (const long int *) regs->lr_sp;

  assert (stack_args[0] == FUNC_STACK_LONG_ARG8);
  assert (stack_args[1] == FUNC_STACK_LONG_ARG9);
  assert (stack_args[2] == FUNC_STACK_LONG_ARG10);
  assert (stack_args[3] == FUNC_STACK_LONG_ARG11);
}

unsigned int
la_version (unsigned int v)
{
  setlinebuf (stdout);
  return v;
}

unsigned int
la_objopen (struct link_map *map, Lmid_t lmid, uintptr_t *cookie)
{
  const char *p = strrchr (map->l_name, '/');
  const char *l_name = p == NULL ? map->l_name : p + 1;
  uintptr_t ck = -1;
  if (l_name[0] == '\0'
      || strncmp (l_name, TEST_NAME, strlen (TEST_NAME)) == 0)
    ck = AUDIT29_COOKIE;
  *cookie = ck;
  printf ("objopen: %ld, %s[%ld]\n", lmid, l_name, (long int) ck);
  return ck == -1 ? 0 : LA_FLG_BINDFROM | LA_FLG_BINDTO;
}

ElfW (Addr)
la_loongarch_gnu_pltenter (ElfW (Sym) *sym, unsigned int ndx,
			   uintptr_t *refcook, uintptr_t *defcook,
			   La_loongarch_regs *regs, unsigned int *flags,
			   const char *symname, long int *framesizep)
{
  printf ("pltenter: symname=%s, st_value=%#lx, ndx=%u, flags=%u\n",
	  symname, (long int) sym->st_value, ndx, *flags);

  if (strcmp (symname, "tst_audit29_func_int") == 0)
    {
      assert ((int) regs->lr_reg[0] == FUNC_INT_ARG0);
      assert ((int) regs->lr_reg[1] == FUNC_INT_ARG1);
      assert ((int) regs->lr_reg[2] == FUNC_INT_ARG2);
      assert ((int) regs->lr_reg[3] == FUNC_INT_ARG3);
      assert ((int) regs->lr_reg[4] == FUNC_INT_ARG4);
      assert ((int) regs->lr_reg[5] == FUNC_INT_ARG5);
      assert ((int) regs->lr_reg[6] == FUNC_INT_ARG6);
      assert ((int) regs->lr_reg[7] == FUNC_INT_ARG7);
    }
  else if (strcmp (symname, "tst_audit29_func_long") == 0)
    {
      assert ((long int) regs->lr_reg[0] == FUNC_LONG_ARG0);
      assert ((long int) regs->lr_reg[1] == FUNC_LONG_ARG1);
      assert ((long int) regs->lr_reg[2] == FUNC_LONG_ARG2);
      assert ((long int) regs->lr_reg[3] == FUNC_LONG_ARG3);
      assert ((long int) regs->lr_reg[4] == FUNC_LONG_ARG4);
      assert ((long int) regs->lr_reg[5] == FUNC_LONG_ARG5);
      assert ((long int) regs->lr_reg[6] == FUNC_LONG_ARG6);
      assert ((long int) regs->lr_reg[7] == FUNC_LONG_ARG7);
    }
  else if (strcmp (symname, "tst_audit29_func_stack_long") == 0)
    {
      assert ((long int) regs->lr_reg[0] == FUNC_LONG_ARG0);
      assert ((long int) regs->lr_reg[1] == FUNC_LONG_ARG1);
      assert ((long int) regs->lr_reg[2] == FUNC_LONG_ARG2);
      assert ((long int) regs->lr_reg[3] == FUNC_LONG_ARG3);
      assert ((long int) regs->lr_reg[4] == FUNC_LONG_ARG4);
      assert ((long int) regs->lr_reg[5] == FUNC_LONG_ARG5);
      assert ((long int) regs->lr_reg[6] == FUNC_LONG_ARG6);
      assert ((long int) regs->lr_reg[7] == FUNC_LONG_ARG7);
      check_stack_long_args (regs);
    }
  else if (strcmp (symname, "tst_audit29_func_ret2") == 0
	   || strcmp (symname, "tst_audit29_func_ret2_modified") == 0)
    {
      /* No input parameters to check.  */
    }
  else if (strcmp (symname, "tst_audit29_func_ret_large") == 0)
    {
      assert (regs->lr_reg[0] != 0);
      assert ((char) regs->lr_reg[1] == FUNC_RET_LARGE_ARG0);
      assert ((short) regs->lr_reg[2] == FUNC_RET_LARGE_ARG1);
      assert ((long int) regs->lr_reg[3] == FUNC_RET_LARGE_ARG2);
    }
#ifndef __loongarch_soft_float
  else if (strcmp (symname, "tst_audit29_func_float") == 0)
    {
      assert (fpreg_float (&regs->lr_vec[0]) == FUNC_FLOAT_ARG0);
      assert (fpreg_float (&regs->lr_vec[1]) == FUNC_FLOAT_ARG1);
      assert (fpreg_float (&regs->lr_vec[2]) == FUNC_FLOAT_ARG2);
      assert (fpreg_float (&regs->lr_vec[3]) == FUNC_FLOAT_ARG3);
      assert (fpreg_float (&regs->lr_vec[4]) == FUNC_FLOAT_ARG4);
      assert (fpreg_float (&regs->lr_vec[5]) == FUNC_FLOAT_ARG5);
      assert (fpreg_float (&regs->lr_vec[6]) == FUNC_FLOAT_ARG6);
      assert (fpreg_float (&regs->lr_vec[7]) == FUNC_FLOAT_ARG7);
    }
  else if (strcmp (symname, "tst_audit29_func_ret2_float") == 0
	   || strcmp (symname, "tst_audit29_func_ret2_float_modified") == 0)
    {
      /* No input parameters to check.  */
    }
#if __loongarch_frlen == 64
  else if (strcmp (symname, "tst_audit29_func_double") == 0)
    {
      assert (fpreg_double (&regs->lr_vec[0]) == FUNC_DOUBLE_ARG0);
      assert (fpreg_double (&regs->lr_vec[1]) == FUNC_DOUBLE_ARG1);
      assert (fpreg_double (&regs->lr_vec[2]) == FUNC_DOUBLE_ARG2);
      assert (fpreg_double (&regs->lr_vec[3]) == FUNC_DOUBLE_ARG3);
      assert (fpreg_double (&regs->lr_vec[4]) == FUNC_DOUBLE_ARG4);
      assert (fpreg_double (&regs->lr_vec[5]) == FUNC_DOUBLE_ARG5);
      assert (fpreg_double (&regs->lr_vec[6]) == FUNC_DOUBLE_ARG6);
      assert (fpreg_double (&regs->lr_vec[7]) == FUNC_DOUBLE_ARG7);
    }
  else if (strcmp (symname, "tst_audit29_func_ret2_double") == 0
	   || strcmp (symname, "tst_audit29_func_ret2_double_modified") == 0)
    {
      /* No input parameters to check.  */
    }
#endif
#endif
  else
    abort ();

#ifndef __loongarch_soft_float
  clobber_fp_regs ();
#endif

  /* A positive size asks the trampoline to reserve and copy a caller stack
     frame.  This keeps stack arguments available to pltexit and also exercises
     the frame-growth path.  */
  *framesizep = 1024;

  return sym->st_value;
}

unsigned int
la_loongarch_gnu_pltexit (ElfW (Sym) *sym, unsigned int ndx,
			  uintptr_t *refcook, uintptr_t *defcook,
			  const La_loongarch_regs *inregs,
			  La_loongarch_retval *outregs, const char *symname)
{
  printf ("pltexit: symname=%s, st_value=%#lx, ndx=%u\n",
	  symname, (long int) sym->st_value, ndx);

  if (strcmp (symname, "tst_audit29_func_int") == 0)
    {
      assert ((int) inregs->lr_reg[0] == FUNC_INT_ARG0);
      assert ((int) inregs->lr_reg[1] == FUNC_INT_ARG1);
      assert ((int) inregs->lr_reg[2] == FUNC_INT_ARG2);
      assert ((int) inregs->lr_reg[3] == FUNC_INT_ARG3);
      assert ((int) inregs->lr_reg[4] == FUNC_INT_ARG4);
      assert ((int) inregs->lr_reg[5] == FUNC_INT_ARG5);
      assert ((int) inregs->lr_reg[6] == FUNC_INT_ARG6);
      assert ((int) inregs->lr_reg[7] == FUNC_INT_ARG7);

      assert ((int) outregs->lrv_a0 == FUNC_INT_RET);
    }
  else if (strcmp (symname, "tst_audit29_func_long") == 0)
    {
      assert ((long int) inregs->lr_reg[0] == FUNC_LONG_ARG0);
      assert ((long int) inregs->lr_reg[1] == FUNC_LONG_ARG1);
      assert ((long int) inregs->lr_reg[2] == FUNC_LONG_ARG2);
      assert ((long int) inregs->lr_reg[3] == FUNC_LONG_ARG3);
      assert ((long int) inregs->lr_reg[4] == FUNC_LONG_ARG4);
      assert ((long int) inregs->lr_reg[5] == FUNC_LONG_ARG5);
      assert ((long int) inregs->lr_reg[6] == FUNC_LONG_ARG6);
      assert ((long int) inregs->lr_reg[7] == FUNC_LONG_ARG7);

      assert ((long int) outregs->lrv_a0 == FUNC_LONG_RET);
    }
  else if (strcmp (symname, "tst_audit29_func_stack_long") == 0)
    {
      assert ((long int) inregs->lr_reg[0] == FUNC_LONG_ARG0);
      assert ((long int) inregs->lr_reg[1] == FUNC_LONG_ARG1);
      assert ((long int) inregs->lr_reg[2] == FUNC_LONG_ARG2);
      assert ((long int) inregs->lr_reg[3] == FUNC_LONG_ARG3);
      assert ((long int) inregs->lr_reg[4] == FUNC_LONG_ARG4);
      assert ((long int) inregs->lr_reg[5] == FUNC_LONG_ARG5);
      assert ((long int) inregs->lr_reg[6] == FUNC_LONG_ARG6);
      assert ((long int) inregs->lr_reg[7] == FUNC_LONG_ARG7);
      check_stack_long_args (inregs);

      assert ((long int) outregs->lrv_a0 == FUNC_STACK_LONG_RET);
    }
  else if (strcmp (symname, "tst_audit29_func_ret2") == 0)
    {
      assert (outregs->lrv_a0 == FUNC_RET2_A0);
      assert (outregs->lrv_a1 == FUNC_RET2_A1);
    }
  else if (strcmp (symname, "tst_audit29_func_ret2_modified") == 0)
    {
      assert (outregs->lrv_a0 == FUNC_RET2_A0);
      assert (outregs->lrv_a1 == FUNC_RET2_A1);

      outregs->lrv_a0 = FUNC_RET2_A0_MODIFIED;
      outregs->lrv_a1 = FUNC_RET2_A1_MODIFIED;
    }
  else if (strcmp (symname, "tst_audit29_func_ret_large") == 0)
    {
      assert (inregs->lr_reg[0] != 0);
      assert (outregs->lrv_a0 == inregs->lr_reg[0]);
      assert ((char) inregs->lr_reg[1] == FUNC_RET_LARGE_ARG0);
      assert ((short) inregs->lr_reg[2] == FUNC_RET_LARGE_ARG1);
      assert ((long int) inregs->lr_reg[3] == FUNC_RET_LARGE_ARG2);
    }
#ifndef __loongarch_soft_float
  else if (strcmp (symname, "tst_audit29_func_float") == 0)
    {
      assert (fpreg_float (&inregs->lr_vec[0]) == FUNC_FLOAT_ARG0);
      assert (fpreg_float (&inregs->lr_vec[1]) == FUNC_FLOAT_ARG1);
      assert (fpreg_float (&inregs->lr_vec[2]) == FUNC_FLOAT_ARG2);
      assert (fpreg_float (&inregs->lr_vec[3]) == FUNC_FLOAT_ARG3);
      assert (fpreg_float (&inregs->lr_vec[4]) == FUNC_FLOAT_ARG4);
      assert (fpreg_float (&inregs->lr_vec[5]) == FUNC_FLOAT_ARG5);
      assert (fpreg_float (&inregs->lr_vec[6]) == FUNC_FLOAT_ARG6);
      assert (fpreg_float (&inregs->lr_vec[7]) == FUNC_FLOAT_ARG7);

      assert (fpreg_float (&outregs->lrv_vec0) == FUNC_FLOAT_RET);
    }
  else if (strcmp (symname, "tst_audit29_func_ret2_float") == 0)
    {
      assert (fpreg_float (&outregs->lrv_vec0) == FUNC_RET2_FP_A0);
      assert (fpreg_float (&outregs->lrv_vec1) == FUNC_RET2_FP_A1);
    }
  else if (strcmp (symname, "tst_audit29_func_ret2_float_modified") == 0)
    {
      assert (fpreg_float (&outregs->lrv_vec0) == FUNC_RET2_FP_A0);
      assert (fpreg_float (&outregs->lrv_vec1) == FUNC_RET2_FP_A1);

      set_fpreg_float (&outregs->lrv_vec0, FUNC_RET2_FP_A0_MODIFIED);
      set_fpreg_float (&outregs->lrv_vec1, FUNC_RET2_FP_A1_MODIFIED);
    }
#if __loongarch_frlen == 64
  else if (strcmp (symname, "tst_audit29_func_double") == 0)
    {
      assert (fpreg_double (&inregs->lr_vec[0]) == FUNC_DOUBLE_ARG0);
      assert (fpreg_double (&inregs->lr_vec[1]) == FUNC_DOUBLE_ARG1);
      assert (fpreg_double (&inregs->lr_vec[2]) == FUNC_DOUBLE_ARG2);
      assert (fpreg_double (&inregs->lr_vec[3]) == FUNC_DOUBLE_ARG3);
      assert (fpreg_double (&inregs->lr_vec[4]) == FUNC_DOUBLE_ARG4);
      assert (fpreg_double (&inregs->lr_vec[5]) == FUNC_DOUBLE_ARG5);
      assert (fpreg_double (&inregs->lr_vec[6]) == FUNC_DOUBLE_ARG6);
      assert (fpreg_double (&inregs->lr_vec[7]) == FUNC_DOUBLE_ARG7);

      assert (fpreg_double (&outregs->lrv_vec0) == FUNC_DOUBLE_RET);
    }
  else if (strcmp (symname, "tst_audit29_func_ret2_double") == 0)
    {
      assert (fpreg_double (&outregs->lrv_vec0) == FUNC_RET2_FP_A0);
      assert (fpreg_double (&outregs->lrv_vec1) == FUNC_RET2_FP_A1);
    }
  else if (strcmp (symname, "tst_audit29_func_ret2_double_modified") == 0)
    {
      assert (fpreg_double (&outregs->lrv_vec0) == FUNC_RET2_FP_A0);
      assert (fpreg_double (&outregs->lrv_vec1) == FUNC_RET2_FP_A1);

      set_fpreg_double (&outregs->lrv_vec0, FUNC_RET2_FP_A0_MODIFIED);
      set_fpreg_double (&outregs->lrv_vec1, FUNC_RET2_FP_A1_MODIFIED);
    }
#endif
#endif
  else
    abort ();

#ifndef __loongarch_soft_float
  clobber_fp_regs ();
#endif

  return 0;
}
