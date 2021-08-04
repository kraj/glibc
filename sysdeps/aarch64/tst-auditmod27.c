/* Check DT_AUDIT for aarch64 specific ABI.
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
#include <assert.h>
#include <link.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "tst-audit.h"
#include "tst-audit27mod.h"

#define TEST_NAME  "tst-audit27"

#define AUDIT27_COOKIE 0

static inline float regs_vec_to_float (const La_aarch64_regs *regs, int i)
{
  float r;
  if (regs->lr_sve == 0)
    r = regs->lr_vreg[i].s;
  else
    memcpy (&r, &regs->lr_vreg[i].z[0], sizeof (r));
  return r;
}

static inline double regs_vec_to_double (const La_aarch64_regs *regs, int i)
{
  double r;
  if (regs->lr_sve == 0)
    r = regs->lr_vreg[i].d;
  else
    memcpy (&r, &regs->lr_vreg[i].z[0], sizeof (r));
  return r;
}

static inline long double regs_vec_to_ldouble (const La_aarch64_regs *regs,
					       int i)
{
  long double r;
  if (regs->lr_sve == 0)
    r = regs->lr_vreg[i].q;
  else
    memcpy (&r, &regs->lr_vreg[i].z[0], sizeof (r));
  return r;
}

static inline float ret_vec_to_float (const La_aarch64_retval *regs, int i)
{
  float r;
  if (regs->lrv_sve == 0)
    r = regs->lrv_vreg[i].s;
  else
    memcpy (&r, &regs->lrv_vreg[i].z[0], sizeof (r));
  return r;
}

static inline double ret_vec_to_double (const La_aarch64_retval *regs, int i)
{
  double r;
  if (regs->lrv_sve == 0)
    r = regs->lrv_vreg[i].d;
  else
    memcpy (&r, &regs->lrv_vreg[i].z[0], sizeof (r));
  return r;
}

static inline long double ret_vec_to_ldouble (const La_aarch64_retval *regs,
					      int i)
{
  long double r;
  if (regs->lrv_sve == 0)
    r = regs->lrv_vreg[i].q;
  else
    memcpy (&r, &regs->lrv_vreg[i].z[0], sizeof (r));
  return r;
}

unsigned int
la_version (unsigned int v)
{
  return v;
}

unsigned int
la_objopen (struct link_map *map, Lmid_t lmid, uintptr_t *cookie)
{
  const char *p = strrchr (map->l_name, '/');
  const char *l_name = p == NULL ? map->l_name : p + 1;
  uintptr_t ck = -1;
  if (strncmp (l_name, TEST_NAME, strlen (TEST_NAME)) == 0)
    ck = AUDIT27_COOKIE;
  *cookie = ck;
  printf ("objopen: %ld, %s [%ld]\n", lmid, l_name, ck);
  return ck == -1 ? 0 : LA_FLG_BINDFROM | LA_FLG_BINDTO;
}


ElfW(Addr)
la_aarch64_gnu_pltenter (ElfW(Sym) *sym, unsigned int ndx, uintptr_t *refcook,
			 uintptr_t *defcook, La_aarch64_regs *regs,
			 unsigned int *flags, const char *symname,
			 long int *framesizep)
{
  printf ("pltenter: symname=%s, st_value=%#lx, ndx=%u, flags=%u\n",
	  symname, (long int) sym->st_value, ndx, *flags);
  printf ("  regs->lr_sve=%d\n", regs->lr_sve);
  if (regs->lr_sve > 0)
    for (int i = 0; i < array_length (regs->lr_vreg); i++)
      printf ("  inregs->lr_vreg[%d]=%p\n", i, regs->lr_vreg[i].z);

  if (strcmp (symname, "tst_audit27_func_float") == 0)
    {
      assert (regs_vec_to_float (regs, 0) == FUNC_FLOAT_ARG0);
      assert (regs_vec_to_float (regs, 1) == FUNC_FLOAT_ARG1);
      assert (regs_vec_to_float (regs, 2) == FUNC_FLOAT_ARG2);
      assert (regs_vec_to_float (regs, 3) == FUNC_FLOAT_ARG3);
      assert (regs_vec_to_float (regs, 4) == FUNC_FLOAT_ARG4);
      assert (regs_vec_to_float (regs, 5) == FUNC_FLOAT_ARG5);
      assert (regs_vec_to_float (regs, 6) == FUNC_FLOAT_ARG6);
      assert (regs_vec_to_float (regs, 7) == FUNC_FLOAT_ARG7);
    }
  else if (strcmp (symname, "tst_audit27_func_double") == 0)
    {
      assert (regs_vec_to_double (regs, 0) == FUNC_DOUBLE_ARG0);
      assert (regs_vec_to_double (regs, 1) == FUNC_DOUBLE_ARG1);
      assert (regs_vec_to_double (regs, 2) == FUNC_DOUBLE_ARG2);
      assert (regs_vec_to_double (regs, 3) == FUNC_DOUBLE_ARG3);
      assert (regs_vec_to_double (regs, 4) == FUNC_DOUBLE_ARG4);
      assert (regs_vec_to_double (regs, 5) == FUNC_DOUBLE_ARG5);
      assert (regs_vec_to_double (regs, 6) == FUNC_DOUBLE_ARG6);
      assert (regs_vec_to_double (regs, 7) == FUNC_DOUBLE_ARG7);
    }
  else if (strcmp (symname, "tst_audit27_func_ldouble") == 0)
    {
      assert (regs_vec_to_ldouble (regs, 0) == FUNC_LDOUBLE_ARG0);
      assert (regs_vec_to_ldouble (regs, 1) == FUNC_LDOUBLE_ARG1);
      assert (regs_vec_to_ldouble (regs, 2) == FUNC_LDOUBLE_ARG2);
      assert (regs_vec_to_ldouble (regs, 3) == FUNC_LDOUBLE_ARG3);
      assert (regs_vec_to_ldouble (regs, 4) == FUNC_LDOUBLE_ARG4);
      assert (regs_vec_to_ldouble (regs, 5) == FUNC_LDOUBLE_ARG5);
      assert (regs_vec_to_ldouble (regs, 6) == FUNC_LDOUBLE_ARG6);
      assert (regs_vec_to_ldouble (regs, 7) == FUNC_LDOUBLE_ARG7);
    }
  else
    abort ();

  /* Clobber the q registers on exit.  */
  uint8_t v = 0xff;
  asm volatile ("dup v0.8b, %w0" : : "r" (v) : "v0");
  asm volatile ("dup v1.8b, %w0" : : "r" (v) : "v1");
  asm volatile ("dup v2.8b, %w0" : : "r" (v) : "v2");
  asm volatile ("dup v3.8b, %w0" : : "r" (v) : "v3");
  asm volatile ("dup v4.8b, %w0" : : "r" (v) : "v4");
  asm volatile ("dup v5.8b, %w0" : : "r" (v) : "v5");
  asm volatile ("dup v6.8b, %w0" : : "r" (v) : "v6");
  asm volatile ("dup v7.8b, %w0" : : "r" (v) : "v7");

  *framesizep = 1024;

  return sym->st_value;
}

unsigned int
la_aarch64_gnu_pltexit (ElfW(Sym) *sym, unsigned int ndx, uintptr_t *refcook,
                        uintptr_t *defcook,
			const struct La_aarch64_regs *inregs,
                        struct La_aarch64_retval *outregs,
			const char *symname)
{
  printf ("pltexit: symname=%s, st_value=%#lx, ndx=%u\n",
	  symname, (long int) sym->st_value, ndx);
  printf ("  inregs->lr_sve=%d\n", inregs->lr_sve);
  if (inregs->lr_sve > 0)
    for (int i = 0; i < array_length (inregs->lr_vreg); i++)
      printf ("  inregs->lr_vreg[%d]=%p\n", i, inregs->lr_vreg[i].z);
  printf ("  outregs->lr_sve=%d\n", outregs->lrv_sve);
  if (outregs->lrv_sve > 0)
    for (int i = 0; i < array_length (outregs->lrv_vreg); i++)
      printf ("  outregs->lr_vreg[%d]=%p\n", i, outregs->lrv_vreg[i].z);

  if (strcmp (symname, "tst_audit27_func_float") == 0)
    {
      assert (regs_vec_to_float (inregs, 0) == FUNC_FLOAT_ARG0);
      assert (regs_vec_to_float (inregs, 1) == FUNC_FLOAT_ARG1);
      assert (regs_vec_to_float (inregs, 2) == FUNC_FLOAT_ARG2);
      assert (regs_vec_to_float (inregs, 3) == FUNC_FLOAT_ARG3);
      assert (regs_vec_to_float (inregs, 4) == FUNC_FLOAT_ARG4);
      assert (regs_vec_to_float (inregs, 5) == FUNC_FLOAT_ARG5);
      assert (regs_vec_to_float (inregs, 6) == FUNC_FLOAT_ARG6);
      assert (regs_vec_to_float (inregs, 7) == FUNC_FLOAT_ARG7);

      assert (ret_vec_to_float (outregs, 0) == FUNC_FLOAT_RET);
    }
  else if (strcmp (symname, "tst_audit27_func_double") == 0)
    {
      assert (regs_vec_to_double (inregs, 0) == FUNC_DOUBLE_ARG0);
      assert (regs_vec_to_double (inregs, 1) == FUNC_DOUBLE_ARG1);
      assert (regs_vec_to_double (inregs, 2) == FUNC_DOUBLE_ARG2);
      assert (regs_vec_to_double (inregs, 3) == FUNC_DOUBLE_ARG3);
      assert (regs_vec_to_double (inregs, 4) == FUNC_DOUBLE_ARG4);
      assert (regs_vec_to_double (inregs, 5) == FUNC_DOUBLE_ARG5);
      assert (regs_vec_to_double (inregs, 6) == FUNC_DOUBLE_ARG6);
      assert (regs_vec_to_double (inregs, 7) == FUNC_DOUBLE_ARG7);

      assert (ret_vec_to_double (outregs, 0) == FUNC_DOUBLE_RET);
    }
  else if (strcmp (symname, "tst_audit27_func_ldouble") == 0)
    {
      assert (regs_vec_to_ldouble (inregs, 0) == FUNC_LDOUBLE_ARG0);
      assert (regs_vec_to_ldouble (inregs, 1) == FUNC_LDOUBLE_ARG1);
      assert (regs_vec_to_ldouble (inregs, 2) == FUNC_LDOUBLE_ARG2);
      assert (regs_vec_to_ldouble (inregs, 3) == FUNC_LDOUBLE_ARG3);
      assert (regs_vec_to_ldouble (inregs, 4) == FUNC_LDOUBLE_ARG4);
      assert (regs_vec_to_ldouble (inregs, 5) == FUNC_LDOUBLE_ARG5);
      assert (regs_vec_to_ldouble (inregs, 6) == FUNC_LDOUBLE_ARG6);
      assert (regs_vec_to_ldouble (inregs, 7) == FUNC_LDOUBLE_ARG7);

      assert (ret_vec_to_ldouble (outregs, 0) == FUNC_LDOUBLE_RET);
    }
  else
    return 0;

  /* Clobber the q registers on exit.  */
  uint8_t v = 0xff;
  asm volatile ("dup v0.8b, %w0" : : "r" (v) : "v0");
  asm volatile ("dup v1.8b, %w0" : : "r" (v) : "v1");
  asm volatile ("dup v2.8b, %w0" : : "r" (v) : "v2");
  asm volatile ("dup v3.8b, %w0" : : "r" (v) : "v3");
  asm volatile ("dup v4.8b, %w0" : : "r" (v) : "v4");
  asm volatile ("dup v5.8b, %w0" : : "r" (v) : "v5");
  asm volatile ("dup v6.8b, %w0" : : "r" (v) : "v6");
  asm volatile ("dup v7.8b, %w0" : : "r" (v) : "v7");

  return 0;
}
