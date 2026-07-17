/* Become root inside a new user namespace.
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

#include <support/namespace.h>

#include <errno.h>
#include <fcntl.h>
#include <intprops.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <support/xunistd.h>
#include <unistd.h>
#ifdef CLONE_NEWNS
# include <sys/mount.h>
#endif /* CLONE_NEWNS */

#if defined CLONE_NEWUSER && defined CLONE_NEWNS
/* Write STR to PATH.  Failure is left to the caller to report, so
   that an unsupported kernel results in a false return from
   support_become_ns_root, not a test failure.  */
static bool
write_string_to_file (const char *path, const char *str)
{
  int fd = open64 (path, O_WRONLY);
  if (fd < 0)
    return false;
  ssize_t len = strlen (str);
  bool ok = write (fd, str, len) == len;
  xclose (fd);
  return ok;
}
#endif /* CLONE_NEWUSER && CLONE_NEWNS */

bool
support_become_ns_root (void)
{
#if defined CLONE_NEWUSER && defined CLONE_NEWNS
  uid_t original_uid = getuid ();
  gid_t original_gid = getgid ();

  if (unshare (CLONE_NEWUSER | CLONE_NEWNS) != 0)
    {
      printf ("warning: unshare (CLONE_NEWUSER | CLONE_NEWNS) failed: %m\n");
      return false;
    }

  /* Map the namespace's UID 0 to the original UID.  Unlike the
     identity mapping support_become_root sets up, this makes the
     process the owner of kernel objects created in the namespace,
     which are owned by its UID 0.  */
  char buf[sizeof "0  1\n" + INT_STRLEN_BOUND (unsigned long long)];
  snprintf (buf, sizeof (buf), "0 %llu 1\n",
            (unsigned long long) original_uid);
  if (!write_string_to_file ("/proc/self/uid_map", buf))
    {
      printf ("warning: writing to /proc/self/uid_map failed: %m\n");
      return false;
    }

  /* Linux 3.19 introduced the setgroups file.  "deny" must be written
     to it before gid_map becomes writable.  */
  if (!write_string_to_file ("/proc/self/setgroups", "deny\n")
      && errno != ENOENT)
    {
      printf ("warning: writing to /proc/self/setgroups failed: %m\n");
      return false;
    }

  /* Now map the namespace's GID 0, like the UID.  */
  snprintf (buf, sizeof (buf), "0 %llu 1\n",
            (unsigned long long) original_gid);
  if (!write_string_to_file ("/proc/self/gid_map", buf))
    {
      printf ("warning: writing to /proc/self/gid_map failed: %m\n");
      return false;
    }

  /* On some systems, / is marked as MS_SHARED, which means that
     mounts within the namespace leak to the rest of the system,
     which is not what we want.  */
  if (mount ("none", "/", NULL, MS_REC | MS_PRIVATE, NULL) != 0)
    {
      printf ("warning: making the mount namespace private failed: %m\n");
      return false;
    }

  return getuid () == 0;
#else
  return false;
#endif /* CLONE_NEWUSER && CLONE_NEWNS */
}
