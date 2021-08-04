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
#include "tst-audit28mod.h"

#define TEST_NAME  "tst-audit28"
#define TEST_FUNC  "tst_audit28_func"

#define AUDIT28_COOKIE 0

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
    ck = AUDIT28_COOKIE;
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


  if (strcmp (symname, TEST_FUNC "_sve_args") == 0)
    {
      svint8_t z0 = svld1_s8 (svptrue_b8  (),
			      (const int8_t *) regs->lr_vreg[0].z);
      svint16_t z1 = svld1_s16 (svptrue_b16 (),
				(const int16_t *) regs->lr_vreg[1].z);
      svint32_t z2 = svld1_s32 (svptrue_b32 (),
				(const int32_t *) regs->lr_vreg[2].z);
      svint64_t z3 = svld1_s64 (svptrue_b64 (),
				(const int64_t *) regs->lr_vreg[3].z);
      svuint8_t z4 = svld1_u8 (svptrue_b8  (),
			       (const uint8_t *) regs->lr_vreg[4].z);
      svuint16_t z5 = svld1_u16 (svptrue_b16 (),
				 (const uint16_t *) regs->lr_vreg[5].z);
      svuint32_t z6 = svld1_u32 (svptrue_b32 (),
				 (const uint32_t *) regs->lr_vreg[6].z);
      svuint64_t z7 = svld1_u64 (svptrue_b64 (),
				 (const uint64_t *) regs->lr_vreg[7].z);
      assert (svptest_any (svptrue_b8  (),  svcmpeq_s8  (svptrue_b8 (),
							 z0, sve_args_z0 ())));
      assert (svptest_any (svptrue_b16 (),  svcmpeq_s16 (svptrue_b16 (),
							 z1, sve_args_z1 ())));
      assert (svptest_any (svptrue_b32 (),  svcmpeq_s32 (svptrue_b32 (),
							 z2, sve_args_z2 ())));
      assert (svptest_any (svptrue_b64 (),  svcmpeq_s64 (svptrue_b64 (),
							 z3, sve_args_z3 ())));
      assert (svptest_any (svptrue_b8  (),  svcmpeq_u8  (svptrue_b8 (),
							 z4, sve_args_z4 ())));
      assert (svptest_any (svptrue_b16 (),  svcmpeq_u16 (svptrue_b16 (),
							 z5, sve_args_z5 ())));
      assert (svptest_any (svptrue_b32 (),  svcmpeq_u32 (svptrue_b32 (),
							 z6, sve_args_z6 ())));
      assert (svptest_any (svptrue_b64 (),  svcmpeq_u64 (svptrue_b64 (),
							 z7, sve_args_z7 ())));
    }
  else
    abort ();

  /* Clobber the q registers on exit.  */
  uint8_t v = 0xff;
  asm volatile ("dup z0.b, %w0" : : "r" (v) : "z0");
  asm volatile ("dup z1.b, %w0" : : "r" (v) : "z1");
  asm volatile ("dup z2.b, %w0" : : "r" (v) : "z2");
  asm volatile ("dup z3.b, %w0" : : "r" (v) : "z3");
  asm volatile ("dup z4.b, %w0" : : "r" (v) : "z4");
  asm volatile ("dup z5.b, %w0" : : "r" (v) : "z5");
  asm volatile ("dup z6.b, %w0" : : "r" (v) : "z6");
  asm volatile ("dup z7.b, %w0" : : "r" (v) : "z7");

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

  if (strcmp (symname, TEST_FUNC "_sve_args") == 0)
    {
      svint8_t z0 = svld1_s8 (svptrue_b8  (),
			      (const int8_t *) inregs->lr_vreg[0].z);
      svint16_t z1 = svld1_s16 (svptrue_b16 (),
				(const int16_t *) inregs->lr_vreg[1].z);
      svint32_t z2 = svld1_s32 (svptrue_b32 (),
				(const int32_t *) inregs->lr_vreg[2].z);
      svint64_t z3 = svld1_s64 (svptrue_b64 (),
				(const int64_t *) inregs->lr_vreg[3].z);
      svuint8_t z4 = svld1_u8 (svptrue_b8  (),
			       (const uint8_t *) inregs->lr_vreg[4].z);
      svuint16_t z5 = svld1_u16 (svptrue_b16 (),
				 (const uint16_t *) inregs->lr_vreg[5].z);
      svuint32_t z6 = svld1_u32 (svptrue_b32 (),
				 (const uint32_t *) inregs->lr_vreg[6].z);
      svuint64_t z7 = svld1_u64 (svptrue_b64 (),
				 (const uint64_t *) inregs->lr_vreg[7].z);
      assert (svptest_any (svptrue_b8  (),  svcmpeq_s8  (svptrue_b8 (),
							 z0, sve_args_z0 ())));
      assert (svptest_any (svptrue_b16 (),  svcmpeq_s16 (svptrue_b16 (),
							 z1, sve_args_z1 ())));
      assert (svptest_any (svptrue_b32 (),  svcmpeq_s32 (svptrue_b32 (),
							 z2, sve_args_z2 ())));
      assert (svptest_any (svptrue_b64 (),  svcmpeq_s64 (svptrue_b64 (),
							 z3, sve_args_z3 ())));
      assert (svptest_any (svptrue_b8  (),  svcmpeq_u8  (svptrue_b8 (),
							 z4, sve_args_z4 ())));
      assert (svptest_any (svptrue_b16 (),  svcmpeq_u16 (svptrue_b16 (),
							 z5, sve_args_z5 ())));
      assert (svptest_any (svptrue_b32 (),  svcmpeq_u32 (svptrue_b32 (),
							 z6, sve_args_z6 ())));
      assert (svptest_any (svptrue_b64 (),  svcmpeq_u64 (svptrue_b64 (),
							 z7, sve_args_z7 ())));

      svint8_t r0 = svld1_s8 (svptrue_b8  (),
			      (const int8_t *) outregs->lrv_vreg[0].z);
      assert (svptest_any (svptrue_b8  (),  svcmpeq_s8  (svptrue_b8 (),
							 r0, sve_ret ())));
    }
  else
    abort ();

  /* Clobber the q registers on exit.  */
  uint8_t v = 0xff;
  asm volatile ("dup z0.b, %w0" : : "r" (v) : "z0");
  asm volatile ("dup z1.b, %w0" : : "r" (v) : "z1");
  asm volatile ("dup z2.b, %w0" : : "r" (v) : "z2");
  asm volatile ("dup z3.b, %w0" : : "r" (v) : "z3");
  asm volatile ("dup z4.b, %w0" : : "r" (v) : "z4");
  asm volatile ("dup z5.b, %w0" : : "r" (v) : "z5");
  asm volatile ("dup z6.b, %w0" : : "r" (v) : "z6");
  asm volatile ("dup z7.b, %w0" : : "r" (v) : "z7");

  return 0;
}
