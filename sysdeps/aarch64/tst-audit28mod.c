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
#include <assert.h>
#include <stdlib.h>
#include <support/check.h>
#include "tst-audit28mod.h"

svint8_t
tst_audit28_func_sve_args (svint8_t z0, svint16_t z1, svint32_t z2,
			   svint64_t z3, svuint8_t z4, svuint16_t z5,
			   svuint32_t z6, svuint64_t z7)
{
  assert (svptest_any (svptrue_b8 (),  svcmpeq_s8  (svptrue_b8 (),
						    z0, sve_args_z0 ())));
  assert (svptest_any (svptrue_b16 (), svcmpeq_s16 (svptrue_b16 (),
						    z1, sve_args_z1 ())));
  assert (svptest_any (svptrue_b32 (), svcmpeq_s32 (svptrue_b32 (),
						    z2, sve_args_z2 ())));
  assert (svptest_any (svptrue_b64 (), svcmpeq_s64 (svptrue_b64 (),
						    z3, sve_args_z3 ())));
  assert (svptest_any (svptrue_b16 (), svcmpeq_u8  (svptrue_b8  (),
						    z4, sve_args_z4 ())));
  assert (svptest_any (svptrue_b16 (), svcmpeq_u16 (svptrue_b16 (),
						    z5, sve_args_z5 ())));
  assert (svptest_any (svptrue_b16 (), svcmpeq_u32 (svptrue_b32 (),
						    z6, sve_args_z6 ())));
  assert (svptest_any (svptrue_b16 (), svcmpeq_u64 (svptrue_b64 (),
						    z7, sve_args_z7 ())));

  return sve_ret ();
}
