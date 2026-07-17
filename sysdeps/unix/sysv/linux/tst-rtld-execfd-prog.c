/* Helper program for tst-rtld-execfd: report argv and AT_EXECFD state.
   Copyright (C) 2026 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

/* Executed by tst-rtld-execfd through a binfmt_misc handler whose
   interpreter is the dynamic linker under test.  Prints what an
   application observes; the parent compares it against a direct
   execution.  */

#include <errno.h>
#include <stdio.h>
#include <sys/auxv.h>

int
main (int argc, char **argv)
{
  printf ("argc=%d\n", argc);
  for (int i = 0; i < argc; ++i)
    printf ("argv[%d]=%s\n", i, argv[i]);

  errno = 0;
  unsigned long int execfd = getauxval (AT_EXECFD);
  printf ("AT_EXECFD=%lu errno=%d\n", execfd, errno);

  const char *execfn = (const char *) getauxval (AT_EXECFN);
  printf ("AT_EXECFN=%s\n", execfn != NULL ? execfn : "(null)");

  return 0;
}
