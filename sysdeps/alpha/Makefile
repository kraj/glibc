# Copyright (C) 1993-2026 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library.  If not, see
# <https://www.gnu.org/licenses/>.

ifeq ($(subdir),debug)
# Consider making this GCC's default...
CFLAGS-backtrace.c = -fasynchronous-unwind-tables
endif

ifeq ($(subdir),gmon)
sysdep_routines += _mcount
endif

ifeq ($(subdir),gnulib)
sysdep_routines += divl divlu divq divqu reml remlu remq remqu
endif

ifeq ($(subdir),string)
sysdep_routines += stxcpy stxncpy
endif

ifeq ($(subdir),stdlib)
sysdep_routines += udiv_qrnnd
endif

ifeq ($(subdir),elf)
# The ld.so startup code cannot use literals until it self-relocates.
CFLAGS-rtld.c = -mbuild-constants
endif

ifeq ($(subdir),math)
# The following routines rely on inexact being raised for correct results.
CFLAGS-s_fma.c = -mieee-with-inexact
CFLAGS-s_fmaf.c = -mieee-with-inexact
CFLAGS-s_llrintf.c += -mieee-with-inexact
CFLAGS-s_llrint.c += -mieee-with-inexact
CFLAGS-s_lrintf.c += -mieee-with-inexact
CFLAGS-s_lrint.c += -mieee-with-inexact
# This test tries to check for inexact being raised by arithmetic.
CFLAGS-test-misc.c += -mieee-with-inexact
# Avoid "conflicting types for built-in function" warnings
CFLAGS-s_isnan.c += -fno-builtin-isnanf

# _Float32x and _Float64 are both binary64 on Alpha, so this narrowing
# divide is a plain divide and the hardware alone decides whether to
# signal underflow.  IEEE 754 determines tininess after rounding from the
# result rounded as if the exponent range were unbounded, but Alpha
# determines it from the delivered result.  For a quotient that is tiny
# but rounds up to the smallest normal -- DBL_MIN / (1 + 2^-52) under a
# rounding mode that rounds away from zero -- the unbounded rounding
# stays below DBL_MIN, because that binade has a finer spacing than the
# subnormals, while the delivered result is DBL_MIN and looks normal.
# Alpha raises no underflow for it.  No trap is taken, so the kernel
# emulation cannot correct this, and the operation has no wider
# intermediate for libm to examine.
test-xfail-test-float32x-float64-div = yes
endif

# Build everything with full IEEE math support, and with dynamic rounding;
# there are a number of math routines that are defined to work with the
# "current" rounding mode, and it's easiest to set this with all of them.
sysdep-CFLAGS += -mieee -mfp-rounding-mode=d

#  Software floating-point emulation.

ifeq ($(subdir),soft-fp)
sysdep_routines += ots_add ots_sub ots_mul ots_div ots_cmp ots_cmpe	\
	ots_cvtxq ots_cvtqx ots_cvtqux ots_cvttx ots_cvtxt ots_nintxq	\
	fraiseexcpt
endif

ifeq ($(subdir),math)
CPPFLAGS += -I../soft-fp
endif
