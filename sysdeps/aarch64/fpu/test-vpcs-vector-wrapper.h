/* Scalar wrapper for vpcs-enabled Advanced SIMD vector math functions.

   Copyright (C) 2023-2026 Free Software Foundation, Inc.
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

#define VPCS_VECTOR_WRAPPER(scalar_func, vector_func)                         \
  extern __attribute__ ((aarch64_vector_pcs))                                 \
  VEC_TYPE vector_func (VEC_TYPE);                                            \
  static void                                                                 \
  CONCAT (scalar_func, _call) (const FLOAT *a, FLOAT *r)                      \
  {                                                                           \
    VEC_TYPE mx, mr;                                                          \
    memcpy (&mx, a, sizeof (mx));                                             \
    mr = vector_func (mx);                                                    \
    memcpy (r, &mr, sizeof (mr));                                             \
  }                                                                           \
  FLOAT scalar_func (FLOAT x)                                                 \
  {                                                                           \
    return mathvec_run_f (CONCAT (scalar_func, _call), x, VEC_LEN);           \
  }

#define VPCS_VECTOR_WRAPPER_ff(scalar_func, vector_func)                      \
  extern __attribute__ ((aarch64_vector_pcs))                                 \
  VEC_TYPE vector_func (VEC_TYPE, VEC_TYPE);                                  \
  static void                                                                 \
  CONCAT (scalar_func, _call) (const FLOAT *a, const FLOAT *b, FLOAT *r)      \
  {                                                                           \
    VEC_TYPE mx, my, mr;                                                      \
    memcpy (&mx, a, sizeof (mx));                                             \
    memcpy (&my, b, sizeof (my));                                             \
    mr = vector_func (mx, my);                                                \
    memcpy (r, &mr, sizeof (mr));                                             \
  }                                                                           \
  FLOAT scalar_func (FLOAT x, FLOAT y)                                        \
  {                                                                           \
    return mathvec_run_ff (CONCAT (scalar_func, _call), x, y, VEC_LEN);       \
  }
