/* Test that LANGUAGE values with path traversal are rejected [BZ #17142].
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

#include <libintl.h>
#include <locale.h>
#include <stdlib.h>
#include <support/check.h>
#include <support/support.h>

static const char *const domaindir = OBJPFX "domaindir";
static const char *const localedir = OBJPFX "domaindir/de_DE";
static const char *const msgdir = OBJPFX "domaindir/de_DE/LC_MESSAGES";

static int
do_test (void)
{
  unsetenv ("OUTPUT_CHARSET");
  /* LANGUAGE is only consulted if the locale is not "C" or "C.<codeset>".
     The catalog is in ISO-8859-1, as is the locale, so no conversion of
     the translation takes place.  */
  xsetlocale (LC_ALL, "de_DE.ISO-8859-1");
  textdomain ("codeset");

  /* Verify that a legitimate LANGUAGE value produces the expected
     translation.  This exercises the normal lookup path and confirms
     that the test .mo catalog is in place.  */
  bindtextdomain ("codeset", domaindir);
  setenv ("LANGUAGE", "de_DE", 1);
  TEST_COMPARE_STRING (gettext ("cheese"), "K\344se");

  /* A bare ".." must be rejected.  Without the fix this would resolve
     to msgdir/../LC_MESSAGES/codeset.mo, which is the real catalog,
     so the translation would succeed.  */
  bindtextdomain ("codeset", msgdir);
  setenv ("LANGUAGE", "..", 1);
  TEST_COMPARE_STRING (gettext ("cheese"), "cheese");

  /* A relative path that leaves and re-enters the catalog directory
     must be rejected.  Without the fix this would resolve to
     localedir/../de_DE/LC_MESSAGES/codeset.mo.  */
  bindtextdomain ("codeset", localedir);
  setenv ("LANGUAGE", "../de_DE", 1);
  TEST_COMPARE_STRING (gettext ("cheese"), "cheese");

  /* Multiple levels of directory traversal must be rejected.  Without
     the fix this would resolve to
     msgdir/../../de_DE/LC_MESSAGES/codeset.mo.  */
  bindtextdomain ("codeset", msgdir);
  setenv ("LANGUAGE", "../../de_DE", 1);
  TEST_COMPARE_STRING (gettext ("cheese"), "cheese");

  /* Invalid entries in a colon-separated LANGUAGE list must be
     skipped individually; valid entries that follow are still
     used.  */
  bindtextdomain ("codeset", domaindir);
  setenv ("LANGUAGE", "..:../de_DE:de_DE", 1);
  TEST_COMPARE_STRING (gettext ("cheese"), "K\344se");

  /* A path containing "/.." must be rejected.  Use localedir so the
     binding changes, which invalidates the DCIGETTEXT result cache.  */
  bindtextdomain ("codeset", localedir);
  setenv ("LANGUAGE", "LC_MESSAGES/..", 1);
  TEST_COMPARE_STRING (gettext ("cheese"), "cheese");

  return 0;
}

#include <support/test-driver.c>
