/* Determine whether string value is affirmation or negative response
   according to current locale's data.
   This file is part of the GNU C Library.
   Copyright (C) 1996-2026 Free Software Foundation, Inc.

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

#include <langinfo.h>
#include <stdlib.h>
#include <regex.h>


/* Match against one of the response patterns, compiling the pattern
   first if necessary.  */
static int
try (const char *response, const int tag, const int match, const int nomatch)
{
  const char *pattern = nl_langinfo (tag);
  regex_t re;
  if (__regcomp (&re, pattern, REG_EXTENDED) != 0)
    return -1;
  int ret = __regexec (&re, response, 0, NULL, 0);
  __regfree (&re);
  switch (ret)
    {
    case 0:
      return match;
    case REG_NOMATCH:
      return nomatch;
    default:
      return -1;
    }
}

int
rpmatch (const char *response)
{
  return (try (response, YESEXPR, 1, 0)
	  ?: try (response, NOEXPR, 0, -1));
}
