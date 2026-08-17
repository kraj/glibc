/* Test that AT_SECURE $ORIGIN rpath entries are looked up using the
   normalized (trusted) path, not the raw expansion (bug 34360).

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


/* For a SUID/SGID program the loader only honors $ORIGIN in DT_RPATH when
   the normalized expansion is rooted in a trusted directory.  If the loader
   opens the un-normalized string (that contains "../"), it might disagree
   as soon as a path component is a symbolic link.

   This test builds the executable with the rpath:

     $ORIGIN/sub/../../../../..SLIBDIR/tst-origin-secure

   and runs it as BASE/a/b/victim, so $ORIGIN is BASE/a/b.  Lexically the
   five "../" pop $ORIGIN/sub back to "/" (BASE is /tmp/tst-origin-secure,
   so $ORIGIN/sub is the five components tmp, tst-origin-secure, a, b, sub),
   and the entry normalizes to the trusted SLIBDIR/tst-origin-secure.  But
   "sub" is a symlink pointing six levels deep under BASE, so opening the raw
   string makes the kernel resolve the "../" through the symlink and land in
   BASE/x1 SLIBDIR/tst-origin-secure instead.

   The "../" count in the rpath (see the Makefile) is therefore
   depth(BASE) + 2 (for the "a/b" of $ORIGIN) + 1 (for "sub"); it is
   independent of SLIBDIR, which is appended whole on both the raw and the
   normalized side.

   A trusted copy of the module (ORIGIN_SECURE_ID_TRUSTED) is installed in
   SLIBDIR/tst-origin-secure; an attacker copy (ORIGIN_SECURE_ID_ATTACKER) is
   placed at the symlink-diverted location.  The trusted subdirectory is
   rooted under SLIBDIR (so it passes the trusted-path check) but is not
   itself a default loader search directory.

   The victim reports, in its exit status, both which module it loaded and
   whether it ran in secure mode.

   Secure mode is forced with glibc.rtld.enable_secure=1 so that no real
   SUID/SGID binary is required.  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xunistd.h>
#include "tst-origin-secure.h"

#define BASE   "/tmp/tst-origin-secure"
#define SONAME "libtst-origin-secure-mod.so"
/* Subdirectory of the trusted SLIBDIR that the rpath normalizes to.  It is
   trusted (rooted under SLIBDIR) but not a default search directory.  */
#define SUBDIR "tst-origin-secure"

static int
run_victim (char *const *envp)
{
  const char *victim = BASE "/a/b/victim";
  char *const argv[] = { (char *) victim, NULL };

  struct support_capture_subprocess res
    = support_capture_subprogram (victim, argv, envp);
  /* The victim itself prints nothing; forward any loader diagnostics to
     the test log.  */
  if (res.err.length > 0)
    printf ("info: victim stderr: %s\n", res.err.buffer);
  int status = res.status;
  support_capture_subprocess_free (&res);
  return WIFEXITED (status) ? WEXITSTATUS (status) : -1;
}

/* With SLIBDIR "/lib64" the container layout is:

     /lib64/tst-origin-secure/libtst-origin-secure-mod.so   trusted copy (id 1)
     BASE/a/b/victim                                        the executable
     BASE/a/b/sub -> BASE/x1/x2/x3/x4/x5/x6                 six levels deep
     BASE/x1/lib64/tst-origin-secure/libtst-origin-secure-mod.so
                                                            attacker copy (id 2)
     BASE/x1/x2/x3/x4/x5/x6/                                the symlink target

   The victim's rpath is $ORIGIN/sub + five "../" + /lib64/tst-origin-secure.  */
static void
do_prepare (int argc, char **argv)
{
  const char *slibdir = support_slibdir_prefix;
  const char *objelf = support_objdir_root;

  char *good_src = xasprintf ("%s/elf/libtst-origin-secure-mod.so", objelf);
  char *evil_src = xasprintf ("%s/elf/tst-origin-secure-evilmod.so", objelf);
  char *victim_src = xasprintf ("%s/elf/tst-origin-secure-victim", objelf);

  xmkdirp (BASE "/a/b", 0755);
  xmkdirp (BASE "/x1/x2/x3/x4/x5/x6", 0755);

  /* Where the trusted copy lives (reached only via the normalized rpath,
     SLIBDIR/SUBDIR) ...  */
  char *good_dir = xasprintf ("%s/%s", slibdir, SUBDIR);
  char *good_dst = xasprintf ("%s/%s", good_dir, SONAME);
  xmkdirp (good_dir, 0755);
  /* ... and where the raw, symlink-diverted lookup lands.  */
  char *evil_dir = xasprintf ("%s/x1%s/%s", BASE, slibdir, SUBDIR);
  char *evil_dst = xasprintf ("%s/%s", evil_dir, SONAME);
  xmkdirp (evil_dir, 0755);

  /* support_copy_file preserves the source mode, so the victim stays
     executable and the modules readable; no chmod is needed.  */
  support_copy_file (good_src, good_dst);
  support_copy_file (evil_src, evil_dst);
  support_copy_file (victim_src, BASE "/a/b/victim");

  unlink (BASE "/a/b/sub");
  xsymlink (BASE "/x1/x2/x3/x4/x5/x6", BASE "/a/b/sub");

  free (good_src);
  free (evil_src);
  free (victim_src);
  free (good_dir);
  free (good_dst);
  free (evil_dir);
  free (evil_dst);
}
#define PREPARE do_prepare

static int
do_test (void)
{
  /* Control run: in normal mode $ORIGIN is honored without the trusted check,
     so the raw rpath resolves through "sub" and the attacker copy is
     loaded.  */
  {
    char *const env[] = { NULL };
    int rc = run_victim (env);
    if (rc != ORIGIN_SECURE_STATUS_ATTACKER)
      FAIL_EXIT1 ("control run returned status %d, expected %d (attacker "
		  "copy, not secure): the $ORIGIN layout does not reproduce "
		  "the divergence between the raw and the normalized rpath",
		  rc, ORIGIN_SECURE_STATUS_ATTACKER);
  }

  /* Secure run: force AT_SECURE.  A fixed loader normalizes the rpath to the
     trusted SLIBDIR/SUBDIR and loads the trusted copy; a loader with the bug
     opens the raw path, resolves "sub", and loads the attacker copy.  */
  {
    char *const env[] = { (char *) "GLIBC_TUNABLES=glibc.rtld.enable_secure=1",
			  NULL };
    int rc = run_victim (env);
    switch (rc)
      {
      /* Secure, trusted copy loaded via the normalized rpath: fixed.  */
      case ORIGIN_SECURE_STATUS_SECURE:
	break;

      /* Secure, attacker copy loaded: the raw rpath was opened.  */
      case ORIGIN_SECURE_STATUS_SECURE | ORIGIN_SECURE_STATUS_ATTACKER:
	FAIL_EXIT1 ("secure-mode loader resolved the un-normalized rpath "
		    "through the attacker symlink (bug 34360)");

      /* Not secure, attacker copy: exactly what the control run produced, so
	 the tunable did not engage and this run says nothing about the
	 trusted-path handling.  */
      case ORIGIN_SECURE_STATUS_ATTACKER:
	FAIL_UNSUPPORTED ("glibc.rtld.enable_secure=1 did not enable "
			  "secure mode (victim status %d)", rc);

      /* Not secure, yet the trusted copy was loaded, which is reachable only
	 through the normalized rpath, and only a secure loader normalizes it.
	 Fail rather than report UNSUPPORTED.  */
      case ORIGIN_SECURE_STATUS_NONE:
	FAIL_EXIT1 ("secure run loaded the trusted copy but the victim "
		    "reports not being secure: __libc_enable_secure is no "
		    "longer a valid proxy for secure mode");

      /* Neither copy loaded: since the trusted copy is reachable only through
	 the normalized rpath, this means the rpath entry was not honored at
	 all.  */
      default:
	FAIL_EXIT1 ("secure run did not load the module via the normalized "
		    "rpath (victim status %d)", rc);
      }
  }

  return 0;
}

#include <support/test-driver.c>
