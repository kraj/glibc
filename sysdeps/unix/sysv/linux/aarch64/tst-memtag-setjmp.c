/* Test for longjmp when we use MTE stack sanitation.
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

#include <setjmp.h>
#include <sys/auxv.h>
#include <stdlib.h>

#include <support/check.h>
#include <support/support.h>
#include <support/xsignal.h>
#include <support/test-driver.h>
#include <support/xthread.h>

#include "tst-mte-helper.h"

static jmp_buf jump_buffer;

/* The longjmp call is an abnormal exit, thus it lets the stack memory
   mte dirty.  No exception is thrown here.  */
static void
__attribute_noinline__
__attribute_optimization_barrier__
foo (void)
{
  volatile unsigned char x[15];
  volatile unsigned char *ptr = &x[0];

  for (int i = 0; i < sizeof (x); ++i)
    ptr[i] = 0xcc;

  longjmp (jump_buffer, 1);  // simulate throw
}

/* Reads data, from a pointer of a given size.  No MTE sanitization
   for this one, any dirty memory should trap here.  */
static void
__attribute_noinline__
hwasan_read (char *p, int size)
{
  volatile char __attribute__ ((unused)) sink;
  for (int i = 0; i < size; ++i)
    sink = p[i];
}

/* Creates a local array, large enough to include any previously tag
   dirty stack memory.  */
static void
__attribute_noinline__
after_catch (void)
{
  char x[10000];
  hwasan_read (&x[0], sizeof(x));
}

static void *
__attribute_noinline__
f (void *closure)
{
  if (setjmp (jump_buffer) == 0)
    foo ();
  else
    after_catch ();

  return NULL;
}

static int
do_test (void)
{
  if (!(getauxval (AT_HWCAP2) & HWCAP2_MTE))
    FAIL_UNSUPPORTED ("kernel or CPU does not support or enable MTE");

  TEST_VERIFY_EXIT (mte_enable ());
  TEST_VERIFY_EXIT (mte_mode () == PR_MTE_TCF_SYNC);

  install_sigsegv_handler_failure ();

  f (NULL);

  xpthread_join (xpthread_create (NULL, f, NULL));

  return 0;
}

#include <support/test-driver.c>
