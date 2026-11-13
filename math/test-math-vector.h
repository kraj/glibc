/* Common definitions for libm tests for vector functions.
   Copyright (C) 2014-2026 Free Software Foundation, Inc.
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
#include <float.h>
#include <stdio.h>
#include <string.h>

#define TEST_MATHVEC 1
#define TEST_NARROW 0
#define TEST_ERRNO 0
#define TEST_EXCEPTIONS 0

#define CNCT(x, y) x ## y
#define CONCAT(a, b) CNCT (a, b)

#define WRAPPER_NAME(function) CONCAT (function, VEC_SUFF)
#define FUNC_TEST(function) WRAPPER_NAME (FUNC (function))

/* The driver runs each test input once for each lane layout, selecting
   the layout in mathvec_layout:

   - The first layouts copy the input to every lane, and each of them
     checks a different lane.

   - The other layouts set the first or the last lane to the input and
     the remaining lanes to one of a few special values, and check the
     lane with the input.  Most vector routines handle these values in a
     separate path, so this catches lanes that are corrupted by the
     special-case handling of other lanes.  */

#define MATHVEC_LAYOUT_DESC_LEN 64
extern int mathvec_layout;
extern int mathvec_num_layouts;
extern char mathvec_layout_desc[MATHVEC_LAYOUT_DESC_LEN];

/* Fill VEC, which has LEN lanes, with VAL according to the current layout
   and return the lane to check.  Also set mathvec_num_layouts, and
   mathvec_layout_desc which the driver appends to the test name.  */
static inline int
mathvec_fill (FLOAT *vec, FLOAT val, int len)
{
  static const struct
  {
    FLOAT value;
    const char *name;
  } others[] =
    {
      { FUNC (__builtin_inf) (), "inf" },
      { -FUNC (__builtin_inf) (), "-inf" },
      { FUNC (__builtin_nan) (""), "qNaN" },
      { -FUNC (__builtin_nan) (""), "-qNaN" },
      { CONCAT (PREFIX, _MAX), "max" },
      { -CONCAT (PREFIX, _MAX), "-max" },
      { 1.0, "1" },
      { -1.0, "-1" },
      { CONCAT (PREFIX, _TRUE_MIN), "min_subnorm" },
      { -CONCAT (PREFIX, _TRUE_MIN), "-min_subnorm" },
      { 0.0, "0" },
      { -0.0, "-0" }
    };
  int layout = mathvec_layout;
  int lane;

  mathvec_num_layouts = len + 2 * array_length (others);
  if (layout < len)
    {
      for (int i = 0; i < len; i++)
	vec[i] = val;
      lane = layout;
      if (lane == 0)
	mathvec_layout_desc[0] = '\0';
      else
	snprintf (mathvec_layout_desc, MATHVEC_LAYOUT_DESC_LEN,
		  " [lane %d]", lane);
    }
  else
    {
      int other = (layout - len) / 2;
      lane = (layout - len) % 2 == 0 ? 0 : len - 1;
      for (int i = 0; i < len; i++)
	vec[i] = others[other].value;
      vec[lane] = val;
      snprintf (mathvec_layout_desc, MATHVEC_LAYOUT_DESC_LEN,
		" [lane %d, other lanes %s]", lane, others[other].name);
    }
  return lane;
}

/* This macro is used in VECTOR_WRAPPER macros for vector tests.  */
#define TEST_VEC_LOOP(vec, len) 				\
  do								\
    {								\
      for (i = 1; i < len; i++)					\
        {							\
          if ((FLOAT) vec[0] != (FLOAT) vec[i])			\
            {							\
              vec[0] = (FLOAT) vec[0] + 0.1;			\
	      break;						\
            }							\
        }							\
    }								\
  while (0)

#define INIT_VEC_LOOP(vec, val, len)				\
  do								\
    {								\
      for (i = 0; i < len; i++)					\
        {							\
          vec[i] = val;						\
        }							\
    }								\
  while (0)

#define WRAPPER_DECL_f(function) extern FLOAT function (FLOAT);
#define WRAPPER_DECL_ff(function) extern FLOAT function (FLOAT, FLOAT);
#define WRAPPER_DECL_fFF(function) extern void function (FLOAT, FLOAT *, FLOAT *);

/* Wrapper from scalar to vector function.  */
#define VECTOR_WRAPPER(scalar_func, vector_func) \
extern VEC_TYPE vector_func (VEC_TYPE);		\
FLOAT scalar_func (FLOAT x)			\
{						\
  FLOAT a[VEC_LEN], r[VEC_LEN];			\
  int lane = mathvec_fill (a, x, VEC_LEN);	\
  VEC_TYPE mx, mr;				\
  memcpy (&mx, a, sizeof (mx));			\
  mr = vector_func (mx);			\
  memcpy (r, &mr, sizeof (mr));			\
  return r[lane];				\
}

/* Wrapper from scalar 2 argument function to vector one.  */
#define VECTOR_WRAPPER_ff(scalar_func, vector_func) 	\
extern VEC_TYPE vector_func (VEC_TYPE, VEC_TYPE);	\
FLOAT scalar_func (FLOAT x, FLOAT y)		\
{						\
  FLOAT a[VEC_LEN], b[VEC_LEN], r[VEC_LEN];	\
  int lane = mathvec_fill (a, x, VEC_LEN);	\
  mathvec_fill (b, y, VEC_LEN);			\
  VEC_TYPE mx, my, mr;				\
  memcpy (&mx, a, sizeof (mx));			\
  memcpy (&my, b, sizeof (my));			\
  mr = vector_func (mx, my);			\
  memcpy (r, &mr, sizeof (mr));			\
  return r[lane];				\
}

/* Wrapper from scalar 3 argument function to vector one.  */
#define VECTOR_WRAPPER_fFF(scalar_func, vector_func) 	\
extern void vector_func (VEC_TYPE, VEC_TYPE *, VEC_TYPE *);	\
void scalar_func (FLOAT x, FLOAT * r, FLOAT * r1)		\
{						\
  int i;					\
  VEC_TYPE mx, mr, mr1;				\
  INIT_VEC_LOOP (mx, x, VEC_LEN);		\
  vector_func (mx, &mr, &mr1);			\
  TEST_VEC_LOOP (mr, VEC_LEN);			\
  TEST_VEC_LOOP (mr1, VEC_LEN);			\
  *r = (FLOAT) mr[0];				\
  *r1 = (FLOAT) mr1[0];				\
  return;					\
}
