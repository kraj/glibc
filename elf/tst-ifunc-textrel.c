/* Check DT_TEXTREL/DF_TEXTREL support with ifunc.
   Copyright (C) 2018 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <stdint.h>

/* Some linkers may fail to produce a shared object with ifunc and text
   relocations.  */
#if CAN_TEXTREL_IFUNC
/* Force a text relocation in the object.  */
static const uintptr_t
address __attribute__((section(".text"))) = (uintptr_t) &address;

static uintptr_t
foo_impl (void)
{
  return address;
}

void *
__attribute__((noinline))
foo (void)
{
  return (void*) foo_impl;
}
__asm__ (".type foo, %gnu_indirect_function");
#endif

static int
do_test (void)
{
#if CAN_TEXTREL_IFUNC
  return (uintptr_t) foo () != 0 ? 0 : 1;
#else
  return 0;
#endif
}

#include <support/test-driver.c>
