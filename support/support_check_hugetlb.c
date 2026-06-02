/* Runtime detection of huge-page support for malloc tests.
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

#include <fcntl.h>
#include <intprops.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <elf/dl-tunables.h>
#include <support/support_check_hugetlb.h>
#include <support/check.h>

bool
support_thp_work_madvise (void)
{
  int fd = open ("/sys/kernel/mm/transparent_hugepage/enabled", O_RDONLY);
  if (fd == -1)
    return false;

#define MODE_MADVISE "always [madvise] never\n"
#define MODE_ALWAYS  "[always] madvise never\n"

  char str[sizeof(MODE_MADVISE)];
  ssize_t s = read (fd, str, sizeof (str));
  close (fd);
  if (s != sizeof (str) - 1)
    return false;
  str[s] = '\0';
  return strcmp (str, MODE_MADVISE) == 0 || strcmp (str, MODE_ALWAYS) == 0;
}

bool
support_hugepages_reserved (void)
{
  int fd = open ("/proc/sys/vm/nr_hugepages", O_RDONLY);
  if (fd == -1)
    return false;

  char str[INT_BUFSIZE_BOUND(unsigned long int)];
  ssize_t s = read (fd, str, sizeof (str));
  close (fd);
  if (s >= sizeof str || s < 0)
    return false;
  str[s] = '\0';
  unsigned long int n = 0;
  return sscanf (str, "%lu", &n) == 1 && n > 0;
}

void
support_check_malloc_hugetlb (void)
{
  if (!TUNABLE_IS_INITIALIZED (glibc, malloc, hugetlb))
    return;

  size_t hugetlb = TUNABLE_GET_FULL (glibc, malloc, hugetlb, size_t, NULL);
  if (hugetlb == 1 && !support_thp_work_madvise ())
    FAIL_UNSUPPORTED ("glibc.malloc.hugetlb=1 requires"
                      " /sys/kernel/mm/transparent_hugepage/enabled"
                      " either always or madvise");
  if (hugetlb == 2 && !support_hugepages_reserved ())
    FAIL_UNSUPPORTED ("glibc.malloc.hugetlb=2 requires"
                      " /proc/sys/vm/nr_hugepages > 0");
}
