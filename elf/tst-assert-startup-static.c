/* Check if assert works during program startup.
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

#include <assert.h>
#include <stdlib.h>

/* The __libc_assert_fail is used internally for assert() calls.  */
extern _Noreturn __typeof (__assert_fail) __libc_assert_fail;

/* The __tunables_init is called just before self-relocation and TLS setup,
   so overriding it is a way to reach the assert code at that point.  */
void
__tunables_init (char **env, char **argv)
{
  /* Inside libc, assert() is redirected to __libc_assert_fail.  This test is
     not built as part of libc, so a plain assert() here would call the public
     __assert_fail instead, which uses __progname and the translation routines
     and thus is not what the startup code issues.  Call the internal routine
     directly.  */
  __libc_assert_fail ("error", __FILE__, __LINE__, __func__);
}

int
main (void)
{
  /* Fail with a different error code than abort.  */
  exit (EXIT_FAILURE);
}
