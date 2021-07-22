/* Check DT_AUDIT la_objopen and la_objclose for all objects.
   Copyright (C) 2021 Free Software Foundation, Inc.
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

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <inttypes.h>
#include <gnu/lib-names.h>
#include <string.h>
#include <stdlib.h>
#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/xstdio.h>
#include <support/xdlfcn.h>
#include <support/support.h>

static int restart;
#define CMDLINE_OPTIONS \
  { "restart", no_argument, &restart, 1 },

static int
handle_restart (void)
{
  xdlopen ("tst-audit23mod.so", RTLD_NOW);
  xdlmopen (LM_ID_NEWLM, LIBC_SO, RTLD_NOW);

  return 0;
}

static inline bool
startswith (const char *str, const char *pre)
{
  size_t lenpre = strlen (pre);
  size_t lenstr = strlen (str);
  return lenstr < lenpre ? false : memcmp (pre, str, lenpre) == 0;
}

static int
do_test (int argc, char *argv[])
{
  /* We must have either:
     - One our fource parameters left if called initially:
       + path to ld.so         optional
       + "--library-path"      optional
       + the library path      optional
       + the application name  */
  if (restart)
    return handle_restart ();

  char *spargv[9];
  int i = 0;
  for (; i < argc - 1; i++)
    spargv[i] = argv[i + 1];
  spargv[i++] = (char *) "--direct";
  spargv[i++] = (char *) "--restart";
  spargv[i] = NULL;

  setenv ("LD_AUDIT", "tst-auditmod23.so", 0);
  struct support_capture_subprocess result
    = support_capture_subprogram (spargv[0], spargv);
  support_capture_subprocess_check (&result, "tst-audit22", 0, sc_allow_stderr);


  /* We expect la_objopen/la_objclose for the objects:
     1. executable
     2. loader
     3. libc.so
     4. tst-audit23mod.so
     5. libc.so (LM_ID_NEWLM)  */
  enum { max_objs = 5 };
  struct la_obj_t
  {
    char *lname;
    uintptr_t laddr;
    Lmid_t lmid;
    bool closed;
  } objs[max_objs] = { [0 ... max_objs-1] = { .closed = false } };
  size_t nobjs = 0;

  FILE *out = fmemopen (result.err.buffer, result.err.length, "r");
  TEST_VERIFY (out != NULL);
  char *buffer = NULL;
  size_t buffer_length = 0;
  while (xgetline (&buffer, &buffer_length, out))
    {
      enum { LA_OBJOPEN, LA_OBJCLOSE} mode;
      ptrdiff_t offset;
      if (startswith (buffer, "la_objopen: "))
	{
	  offset = strlen ("la_objopen: ");
	  mode = LA_OBJOPEN;
	}
      else if (startswith (buffer, "la_objclose: "))
	{
	  offset = strlen ("la_objclose: ");
	  mode = LA_OBJCLOSE;
	}
      else
	continue;

      char *lname;
      uintptr_t laddr;
      Lmid_t lmid;
      int r = sscanf (buffer + offset, "%ms %"SCNxPTR" %ld", &lname, &laddr,
		      &lmid);
      TEST_COMPARE (r, 3);

      if (mode == LA_OBJOPEN)
	{
	  if (nobjs == max_objs)
	    FAIL_EXIT1 ("non expected la_objopen: %s %"PRIxPTR" %ld",
			lname, laddr, lmid);
	  objs[nobjs].lname = lname;
	  objs[nobjs].laddr = laddr;
	  objs[nobjs].lmid = lmid;
	  objs[nobjs].closed = false;
	  nobjs++;
	}
      else if (mode == LA_OBJCLOSE)
	{
	  for (size_t i = 0; i < nobjs; i++)
	    {
	      if (strcmp (lname, objs[i].lname) == 0 && lmid == objs[i].lmid)
		{
		  TEST_COMPARE (objs[i].closed, false);
		  objs[i].closed = true;
		  break;
		}
	    }
	}
    }

  for (size_t i = 0; i < nobjs; i++)
    {
      TEST_COMPARE (objs[i].closed, true);
      free (objs[i].lname);
    }

  free (buffer);
  xfclose (out);

  return 0;
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
