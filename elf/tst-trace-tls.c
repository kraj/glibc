/* Check that trace mode handles unresolved TLS symbols (BZ 34532).
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/subprocess.h>
#include <support/support.h>

/* Check if unresolved TLS symbol in trace mode (LD_TRACE_LOADED_OBJECTS)
   is correctly reported.  */

static char *mod;
static char *libc_path;

static struct support_capture_subprocess
run_ldso (const char *env1, const char *env2)
{
  char *const argv[] = { mod, NULL };
  char *const envp[] =
    { (char *) "LD_TRACE_LOADED_OBJECTS=1", (char *) env1, (char *) env2,
      NULL };

  struct support_spawn_wrapped *wrapped
    = support_spawn_wrap (mod, argv, envp, support_spawn_wrap_force);
  struct support_capture_subprocess result
    = support_capture_subprogram (wrapped->path, wrapped->argv,
				  wrapped->envp);
  support_spawn_wrapped_free (wrapped);

  support_capture_subprocess_check (&result, "tst-trace-tls", 0,
				    sc_allow_stdout | sc_allow_stderr);
  return result;
}

static void
run_trace (const char *title, const char *env1, const char *env2,
	   bool check_undefined)
{
  printf ("info: checking %s\n", title);

  struct support_capture_subprocess result = run_ldso (env1, env2);

  if (check_undefined)
    {
      /* The dependencies must come from the build tree.  */
      TEST_VERIFY (strstr (result.out.buffer, libc_path) != NULL);
      TEST_VERIFY (strstr (result.err.buffer,
			   "undefined symbol: missing_tls_gd") != NULL);
      TEST_VERIFY (strstr (result.err.buffer,
			   "undefined symbol: missing_tls_ie") != NULL);
      TEST_VERIFY (strstr (result.err.buffer,
			   "undefined symbol: missing_data") != NULL);
    }

  support_capture_subprocess_free (&result);
}

static int
do_test (void)
{
  mod = xasprintf ("%s/elf/tst-trace-tls-mod.so", support_objdir_root);
  libc_path = xasprintf ("=> %s/libc.so.6 ", support_objdir_root);

  run_trace ("ldd -d", "LD_WARN=yes", NULL, true);
  run_trace ("ldd -r", "LD_WARN=yes", "LD_BIND_NOW=1", true);
  run_trace ("ldd -u", "LD_DEBUG=unused", NULL, false);

  free (libc_path);
  free (mod);

  return 0;
}

#include <support/test-driver.c>
