/* Test the ld.so --program-fd option.
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

/* Run the dynamic linker with --program-fd on an inherited descriptor
   of the helper program: it must be loaded from the descriptor, with
   the program name argument only naming it.  Exercises the same
   loading path as AT_EXECFD, without requiring kernel support.  */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xunistd.h>

static int
do_test (void)
{
  char *prog = xasprintf ("%s/elf/tst-rtld-program-fd-prog",
			  support_objdir_root);
  char *libpath = xasprintf ("%s:%s/elf", support_objdir_root,
			     support_objdir_root);

  /* No O_CLOEXEC: the descriptor must survive into ld.so.  */
  int fd = xopen (prog, O_RDONLY, 0);
  char *fdstr = xasprintf ("%d", fd);

  /* Plain use: the program comes from the descriptor, the name
     argument becomes argv[0].  */
  {
    char *argv[] =
      {
	(char *) "ld.so", (char *) "--library-path", libpath,
	(char *) "--program-fd", fdstr,
	(char *) "displayed-name", (char *) "tail-arg", NULL
      };
    struct support_capture_subprocess cap
      = support_capture_subprogram (support_objdir_elf_ldso, argv, NULL);
    support_capture_subprocess_check (&cap, "program-fd", 0,
				      sc_allow_stdout);
    TEST_COMPARE_STRING (cap.out.buffer,
			 "argc=2\n"
			 "argv[0]=displayed-name\n"
			 "argv[1]=tail-arg\n");
    support_capture_subprocess_free (&cap);
  }

  /* The descriptor's file position must be irrelevant and preserved:
     the loader reads the ELF header with pread.  Hand over the
     descriptor deliberately positioned mid-file and verify that the
     program still runs and that the position - shared with the
     subprocess - is where it was left.  */
  {
    xlseek (fd, 123, SEEK_SET);
    char *argv[] =
      {
	(char *) "ld.so", (char *) "--library-path", libpath,
	(char *) "--program-fd", fdstr,
	(char *) "displayed-name", NULL
      };
    struct support_capture_subprocess cap
      = support_capture_subprogram (support_objdir_elf_ldso, argv, NULL);
    support_capture_subprocess_check (&cap, "program-fd position", 0,
				      sc_allow_stdout);
    TEST_COMPARE_STRING (cap.out.buffer,
			 "argc=1\n"
			 "argv[0]=displayed-name\n");
    TEST_COMPARE (xlseek (fd, 0, SEEK_CUR), 123);
    support_capture_subprocess_free (&cap);
  }

  /* Composes with --argv0.  No rewind: the previous subtest left the
     position mid-file, which the loader ignores.  */
  {
    char *argv[] =
      {
	(char *) "ld.so", (char *) "--library-path", libpath,
	(char *) "--program-fd", fdstr, (char *) "--argv0",
	(char *) "overridden", (char *) "displayed-name", NULL
      };
    struct support_capture_subprocess cap
      = support_capture_subprogram (support_objdir_elf_ldso, argv, NULL);
    support_capture_subprocess_check (&cap, "program-fd --argv0", 0,
				      sc_allow_stdout);
    TEST_COMPARE_STRING (cap.out.buffer,
			 "argc=1\n"
			 "argv[0]=overridden\n");
    support_capture_subprocess_free (&cap);
  }

  /* A closed descriptor must produce a clean error, not a crash.  */
  {
    char *argv[] =
      {
	(char *) "ld.so", (char *) "--program-fd", (char *) "977",
	(char *) "does-not-matter", NULL
      };
    struct support_capture_subprocess cap
      = support_capture_subprogram (support_objdir_elf_ldso, argv, NULL);
    support_capture_subprocess_check (&cap, "program-fd bad fd", 127,
				      sc_allow_stderr);
    TEST_VERIFY (strstr (cap.err.buffer,
			 "cannot load main program from descriptor")
		 != NULL);
    support_capture_subprocess_free (&cap);
  }

  /* A non-numeric argument must produce a clean error.  */
  {
    char *argv[] =
      {
	(char *) "ld.so", (char *) "--program-fd", (char *) "pear",
	(char *) "does-not-matter", NULL
      };
    struct support_capture_subprocess cap
      = support_capture_subprogram (support_objdir_elf_ldso, argv, NULL);
    support_capture_subprocess_check (&cap, "program-fd non-numeric", 127,
				      sc_allow_stderr);
    TEST_VERIFY (strstr (cap.err.buffer, "invalid descriptor") != NULL);
    support_capture_subprocess_free (&cap);
  }

  xclose (fd);
  free (fdstr);
  free (libpath);
  free (prog);
  return 0;
}

#include <support/test-driver.c>
