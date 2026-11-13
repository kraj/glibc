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
     the remaining lanes to one of a few control values, and check the
     lane with the input.  Most vector routines handle these values in a
     separate path, so this catches lanes that are corrupted by the
     special-case handling of other lanes.  The routine is also called
     with the control value in every lane, and the control lanes must
     give the same results in both calls, so the input lane must not
     corrupt them either.

   The wrappers call mathvec_run_f or mathvec_run_ff, which fill the
   vectors for the current layout, call the routine and return the
   result in the checked lane.  They also set mathvec_num_layouts,
   mathvec_layout_desc, which the driver appends to the test name, and
   mathvec_changed, the control lanes whose results differ between the
   two calls, which the driver checks with the ulps of the routine.  */

#define MATHVEC_LAYOUT_DESC_LEN 64
extern int mathvec_layout;
extern int mathvec_num_layouts;
extern char mathvec_layout_desc[MATHVEC_LAYOUT_DESC_LEN];

/* The most lanes of any vector type.  */
#define MATHVEC_MAX_LANES 64

/* A control lane whose result changed when the input was put in the
   checked lane.  */
struct mathvec_changed
{
  /* The index of the control lane.  */
  int lane;
  /* Its result with the input in the checked lane.  */
  FLOAT result;
  /* Its result with the control value in every lane.  */
  FLOAT expected;
};
extern struct mathvec_changed mathvec_changed[MATHVEC_MAX_LANES];
extern int mathvec_num_changed;

/* A control value, which a layout puts in every lane but the checked
   one.  */
struct mathvec_control
{
  FLOAT value;
  /* How the test name shows the value, for example "inf" in
     "[lane 0, other lanes inf]".  */
  const char *name;
};

/* Return the control value of the current layout for a vector with LEN lanes,
   or NULL if the layout copies the input to every lane.  */
static inline const struct mathvec_control *
mathvec_get_control (int len)
{
  static const struct mathvec_control controls[] =
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

  mathvec_num_layouts = len + 2 * array_length (controls);
  if (mathvec_layout < len)
    return NULL;
  return &controls[(mathvec_layout - len) / 2];
}

/* Fill VEC, which has LEN lanes, with VAL according to the current layout
   and return the lane to check.  */
static inline int
mathvec_fill (FLOAT *vec, FLOAT val, int len)
{
  const struct mathvec_control *control = mathvec_get_control (len);
  int lane;

  mathvec_num_changed = 0;
  if (control == NULL)
    {
      for (int i = 0; i < len; i++)
	vec[i] = val;
      lane = mathvec_layout;
      if (lane == 0)
	mathvec_layout_desc[0] = '\0';
      else
	snprintf (mathvec_layout_desc, MATHVEC_LAYOUT_DESC_LEN,
		  " [lane %d]", lane);
    }
  else
    {
      lane = (mathvec_layout - len) % 2 == 0 ? 0 : len - 1;
      for (int i = 0; i < len; i++)
	vec[i] = control->value;
      vec[lane] = val;
      snprintf (mathvec_layout_desc, MATHVEC_LAYOUT_DESC_LEN,
		" [lane %d, other lanes %s]", lane, control->name);
    }
  return lane;
}

/* Fill VEC, which has LEN lanes, with the control value of the current
   layout.  Return 0 if the layout has no control value.  */
static inline int
mathvec_fill_control (FLOAT *vec, int len)
{
  const struct mathvec_control *control = mathvec_get_control (len);

  if (control == NULL)
    return 0;
  for (int i = 0; i < len; i++)
    vec[i] = control->value;
  return 1;
}

/* Record in mathvec_changed the lanes other than LANE where RESULT and
   EXPECTED, which have LEN lanes, differ.  Any two NaNs are equal.  */
static inline void
mathvec_compare_control (const FLOAT *result, const FLOAT *expected,
			 int lane, int len)
{
  for (int i = 0; i < len && mathvec_num_changed < MATHVEC_MAX_LANES; i++)
    if (i != lane
	&& !(__builtin_isnan (result[i]) && __builtin_isnan (expected[i]))
	&& memcmp (&result[i], &expected[i], sizeof (FLOAT)) != 0)
      mathvec_changed[mathvec_num_changed++]
	= (struct mathvec_changed) { i, result[i], expected[i] };
}

/* Run a routine with one argument for the current layout of a vector
   with LEN lanes and input X.  CALL loads A into a vector, calls the
   routine and stores the result in R.  */
static inline FLOAT
mathvec_run_f (void (*call) (const FLOAT *a, FLOAT *r), FLOAT x, int len)
{
  FLOAT a[len], r[len];
  int lane = mathvec_fill (a, x, len);

  call (a, r);
  if (mathvec_fill_control (a, len))
    {
      FLOAT rc[len];
      call (a, rc);
      mathvec_compare_control (r, rc, lane, len);
    }
  return r[lane];
}

/* Likewise for a routine with two arguments and inputs X and Y.  */
static inline FLOAT
mathvec_run_ff (void (*call) (const FLOAT *a, const FLOAT *b, FLOAT *r),
		FLOAT x, FLOAT y, int len)
{
  FLOAT a[len], b[len], r[len];
  int lane = mathvec_fill (a, x, len);

  mathvec_fill (b, y, len);
  call (a, b, r);
  if (mathvec_fill_control (a, len))
    {
      FLOAT rc[len];
      mathvec_fill_control (b, len);
      call (a, b, rc);
      mathvec_compare_control (r, rc, lane, len);
    }
  return r[lane];
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
#define VECTOR_WRAPPER(scalar_func, vector_func)			\
extern VEC_TYPE vector_func (VEC_TYPE);					\
static void								\
CONCAT (scalar_func, _call) (const FLOAT *a, FLOAT *r)			\
{									\
  VEC_TYPE mx, mr;							\
  memcpy (&mx, a, sizeof (mx));						\
  mr = vector_func (mx);						\
  memcpy (r, &mr, sizeof (mr));						\
}									\
FLOAT scalar_func (FLOAT x)						\
{									\
  return mathvec_run_f (CONCAT (scalar_func, _call), x, VEC_LEN);	\
}

/* Wrapper from scalar 2 argument function to vector one.  */
#define VECTOR_WRAPPER_ff(scalar_func, vector_func)			\
extern VEC_TYPE vector_func (VEC_TYPE, VEC_TYPE);			\
static void								\
CONCAT (scalar_func, _call) (const FLOAT *a, const FLOAT *b, FLOAT *r)	\
{									\
  VEC_TYPE mx, my, mr;							\
  memcpy (&mx, a, sizeof (mx));						\
  memcpy (&my, b, sizeof (my));						\
  mr = vector_func (mx, my);						\
  memcpy (r, &mr, sizeof (mr));						\
}									\
FLOAT scalar_func (FLOAT x, FLOAT y)					\
{									\
  return mathvec_run_ff (CONCAT (scalar_func, _call), x, y, VEC_LEN);	\
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
