/* Get file-specific information about a file.  Linux/ia64 version.
   Copyright (C) 2003-2019 Free Software Foundation, Inc.
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

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <not-cancel.h>

static bool itc_usable;

static bool
ia64_check_cpuclock (void)
{
  if (__glibc_unlikely (itc_usable == 0))
    {
      int newval = true;
      int fd = __open_nocancel ("/proc/sal/itc_drift", O_RDONLY);
      if (__glibc_likely (fd != -1))
	{
	  char buf[16];
	  /* We expect the file to contain a single digit followed by
	     a newline.  If the format changes we better not rely on
	     the file content.  */
	  if (__read_nocancel (fd, buf, sizeof buf) != 2
	      || buf[0] != '0' || buf[1] != '\n')
	    newval = false;

	  __close_nocancel_nostatus (fd);
	}

      itc_usable = newval;
    }

  return itc_usable;
}
#define HAS_CPUCLOCK(name) (ia64_check_cpuclock () ? _POSIX_VERSION : -1)

/* Now the generic Linux version.  */
#include <sysdeps/unix/sysv/linux/sysconf.c>
