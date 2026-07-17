/* Test that ld.so loads the main program from AT_EXECFD.
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

/* Register the dynamic linker under test as a binfmt_misc extension
   handler with the 'O' (open-binary) flag in a private user and mount
   namespace, execute a helper through it, and verify that the helper
   was loaded from the AT_EXECFD descriptor: the application-visible
   argument vector is unchanged, the descriptor is not observable via
   getauxval, LD_TRACE_LOADED_OBJECTS works, and an execute-only (--x)
   copy - unopenable by path - still runs.

   Requires a kernel with per-user-namespace binfmt_misc mounts
   (Linux >= 6.7); reports UNSUPPORTED otherwise.  */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <linux/capability.h>

#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/namespace.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/xunistd.h>

/* The binfmt_misc extension (filename suffix) the handler matches.  */
#define EXT "tstexecfd"

static char *binfmt_dir;
static char *prog_copy;
static char *libpath_env;

/* Unmount the binfmt_misc instance.  Registered with atexit so that a
   failing subtest does not leave the mount pinning the temporary
   directory; the temporary files are deleted after this runs (atexit
   handlers run in reverse registration order).  */
static void
unmount_binfmt (void)
{
  umount2 (binfmt_dir, MNT_DETACH);
}

/* Drop the DAC-bypassing capabilities so that file permissions apply
   to this (namespace-root) process again.  Cannot be undone.  */
static void
drop_dac_capabilities (void)
{
  struct __user_cap_header_struct header =
    { .version = _LINUX_CAPABILITY_VERSION_3, .pid = 0 };
  struct __user_cap_data_struct data[2];
  TEST_COMPARE (syscall (SYS_capget, &header, data), 0);
  data[0].effective &= ~((1u << CAP_DAC_OVERRIDE) | (1u << CAP_DAC_READ_SEARCH));
  data[0].permitted &= ~((1u << CAP_DAC_OVERRIDE) | (1u << CAP_DAC_READ_SEARCH));
  data[0].inheritable &= ~((1u << CAP_DAC_OVERRIDE)
			   | (1u << CAP_DAC_READ_SEARCH));
  TEST_COMPARE (syscall (SYS_capset, &header, data), 0);

  /* capset leaves the bounding set untouched, and a namespace-root
     execve re-grants permitted = bounding | inheritable (effective
     too), so the exec'd loader would regain CAP_DAC_OVERRIDE and could
     open the execute-only file by path.  Drop the caps from the
     bounding set as well so file permissions bind across the exec.  */
  TEST_COMPARE (prctl (PR_CAPBSET_DROP, CAP_DAC_OVERRIDE, 0, 0, 0), 0);
  TEST_COMPARE (prctl (PR_CAPBSET_DROP, CAP_DAC_READ_SEARCH, 0, 0, 0), 0);
}

static struct support_capture_subprocess
run_prog_copy (const char *arg1, const char *arg2, bool trace)
{
  char *argv[] = { prog_copy, (char *) arg1, (char *) arg2, NULL };
  char *envp[3] = { libpath_env, NULL, NULL };
  if (trace)
    envp[1] = (char *) "LD_TRACE_LOADED_OBJECTS=1";
  return support_capture_subprogram (prog_copy, argv, envp);
}

static int
do_test (void)
{
  /* The namespace's uid/gid 0 must be mapped (support_become_root
     identity-maps the original ids instead): the binfmt_misc inodes
     are owned by the namespace's uid 0, and an unmapped owner cannot
     pass the permission (or capability) checks for writing to the
     register file.  */
  if (!support_become_ns_root ())
    FAIL_UNSUPPORTED ("cannot create user+mount namespace with uid 0");

  /* A binfmt_misc instance mounted in a user namespace is private to
     it: registrations neither affect nor require anything from the
     host (Linux >= 6.7).  */
  binfmt_dir = support_create_temp_directory ("tst-rtld-execfd-binfmt-");
  if (mount ("binfmt_misc", binfmt_dir, "binfmt_misc", 0, NULL) != 0)
    {
      if (errno == ENODEV || errno == ENOENT || errno == ENOSYS
	  || errno == EPERM || errno == EACCES || errno == EINVAL)
	FAIL_UNSUPPORTED ("cannot mount binfmt_misc: %m");
      FAIL_EXIT1 ("mount binfmt_misc: %m");
    }
  atexit (unmount_binfmt);

  /* Register the dynamic linker under test as an extension handler
     with the 'O' flag, so the kernel keeps the executed binary open
     and passes it in AT_EXECFD.  */
  {
    char *reg = xasprintf (":tst-rtld-execfd:E::" EXT "::%s:O",
			   support_objdir_elf_ldso);
    char *regpath = xasprintf ("%s/register", binfmt_dir);
    support_write_file_string (regpath, reg);
    free (regpath);
    free (reg);
  }

  /* The handler execs ld.so without options, so the helper must find
     the build-tree libraries through the environment.  */
  libpath_env = xasprintf ("LD_LIBRARY_PATH=%s:%s/elf",
			   support_objdir_root, support_objdir_root);

  char *prog = xasprintf ("%s/elf/tst-rtld-execfd-prog",
			  support_objdir_root);
  char *tmpdir = support_create_temp_directory ("tst-rtld-execfd-");
  prog_copy = xasprintf ("%s/prog." EXT, tmpdir);
  support_copy_file (prog, prog_copy);
  add_temp_file (prog_copy);
  free (tmpdir);
  free (prog);

  /* Execute the helper through the handler.  The application must
     observe exactly what a direct execution would produce: the argv
     the kernel spliced for the interpreter is consumed by ld.so as
     usual, and the descriptor is loaded from and neutralized.  */
  {
    struct support_capture_subprocess cap
      = run_prog_copy ("first-arg", "second-arg", false);
    support_capture_subprocess_check (&cap, "execfd", 0, sc_allow_stdout);
    char *expected = xasprintf ("argc=3\n"
				"argv[0]=%s\n"
				"argv[1]=first-arg\n"
				"argv[2]=second-arg\n"
				"AT_EXECFD=0 errno=%d\n"
				"AT_EXECFN=%s\n",
				prog_copy, ENOENT, prog_copy);
    TEST_COMPARE_STRING (cap.out.buffer, expected);
    free (expected);
    support_capture_subprocess_free (&cap);
  }

  /* LD_TRACE_LOADED_OBJECTS (ldd) must work for a descriptor-loaded
     main program.  */
  {
    struct support_capture_subprocess cap
      = run_prog_copy (NULL, NULL, true);
    support_capture_subprocess_check (&cap, "execfd trace", 0,
				      sc_allow_stdout);
    TEST_VERIFY (strstr (cap.out.buffer, "libc.so") != NULL);
    support_capture_subprocess_free (&cap);
  }

  /* A program whose spliced path begins with "--" must be run, not
     mistaken for a dynamic-linker option: the kernel copies the
     execve() pathname into the interpreter's argument vector verbatim,
     so a program named like a loader option would otherwise divert or
     abort the loader.  A relative path is required for the kernel to
     hand ld.so an argument starting with "--"; an absolute path always
     begins with "/".  */
  {
    const char *dashrel = "--library-path." EXT;
    char *dashdir = support_create_temp_directory ("tst-rtld-execfd-dash-");
    char *dashabs = xasprintf ("%s/%s", dashdir, dashrel);
    support_copy_file (prog_copy, dashabs);
    add_temp_file (dashabs);

    xchdir (dashdir);
    char *argv[] = { (char *) dashrel, (char *) "tail", NULL };
    char *envp[2] = { libpath_env, NULL };
    struct support_capture_subprocess cap
      = support_capture_subprogram (dashrel, argv, envp);
    xchdir ("/");
    support_capture_subprocess_check (&cap, "execfd dashname", 0,
				      sc_allow_stdout);
    char *expected = xasprintf ("argc=2\n"
				"argv[0]=%s\n"
				"argv[1]=tail\n"
				"AT_EXECFD=0 errno=%d\n"
				"AT_EXECFN=%s\n",
				dashrel, ENOENT, dashrel);
    TEST_COMPARE_STRING (cap.out.buffer, expected);
    free (expected);
    support_capture_subprocess_free (&cap);
    free (dashabs);
    free (dashdir);
  }

  /* The headline capability: an execute-only binary cannot be opened
     by path, but execve() permits it and the descriptor the kernel
     passes is readable.  Give up the DAC-override capabilities first,
     otherwise this (namespace-root) process could open it anyway.
     This must be the last subtest: the capabilities are gone.  */
  {
    TEST_COMPARE (chmod (prog_copy, 0111), 0);
    drop_dac_capabilities ();

    /* Control: the path really is unopenable now.  This is what any
       path-based re-open in ld.so would run into.  */
    errno = 0;
    TEST_COMPARE (open (prog_copy, O_RDONLY), -1);
    TEST_COMPARE (errno, EACCES);

    struct support_capture_subprocess cap
      = run_prog_copy ("x-only", NULL, false);
    support_capture_subprocess_check (&cap, "execfd execute-only", 0,
				      sc_allow_stdout);
    TEST_VERIFY (strstr (cap.out.buffer, "argv[1]=x-only") != NULL);
    TEST_VERIFY (strstr (cap.out.buffer, "AT_EXECFD=0 errno=2") != NULL);
    support_capture_subprocess_free (&cap);
  }

  return 0;
}

#include <support/test-driver.c>
