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

#ifndef _TST_AUDIT27MOD_H
#define _TST_AUDIT27MOD_H 1

#include <arm_sve.h>

static inline svint8_t sve_args_z0 (void)
{
  return svdup_s8 (INT8_MAX);
}

static inline svint16_t sve_args_z1 (void)
{
  return svdup_s16 (INT16_MAX);
}

static inline svint32_t sve_args_z2 (void)
{
  return svdup_s32 (INT32_MAX);
}

static inline svint64_t sve_args_z3 (void)
{
  return svdup_s64 (INT64_MAX);
}

static inline svuint8_t sve_args_z4 (void)
{
  return svdup_u8 (UINT8_MAX);
}

static inline svuint16_t sve_args_z5 (void)
{
  return svdup_u16 (UINT16_MAX);
}

static inline svuint32_t sve_args_z6 (void)
{
  return svdup_u32 (UINT32_MAX);
}

static inline svuint64_t sve_args_z7 (void)
{
  return svdup_u64 (UINT64_MAX);
}

static inline svint8_t sve_ret (void)
{
  return svdup_s8 (INT8_MIN);
}

#define INT_ARGS_RET 0x21

svint8_t tst_audit28_func_sve_args (svint8_t z0, svint16_t z1, svint32_t z2, svint64_t z3,
				    svuint8_t z4, svuint16_t z5, svuint32_t z6, svuint64_t z7);

#endif
