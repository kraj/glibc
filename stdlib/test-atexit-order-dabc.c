/* Check atexit execution order regarding other exit types.
   Copyright (C) 2019 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <errno.h>

extern void set_a_exit_code (int);

int main (int argc, char *argv[])
{
  if (argc < 2)
    exit (EXIT_FAILURE);

  int exit_code = strtol (argv[1], NULL, 10);
  if (errno == ERANGE || exit_code < 0 || exit_code > 255)
    exit (EXIT_FAILURE);

  set_a_exit_code (exit_code);

  exit (0);

  return 0;
}
