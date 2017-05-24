/* Common glob test definition.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

// #define DEBUG
#ifdef DEBUG
# define PRINTF(fmt, args...) \
  do					\
    {					\
      int save_errno = errno;		\
      printf (fmt, ##args);		\
      errno = save_errno;		\
    } while (0)
#else
# define PRINTF(fmt, args...)
#endif

struct filesystem_t
{
  const char *name;
  int level;
  int type;
  mode_t mode;
};

static long int
find_file (const char *s, const struct filesystem_t *filesystem,
	   size_t nfiles)
{
  int level = 1;
  long int idx = 0;

  while (s[0] == '/')
    {
      if (s[1] == '\0')
	{
	  s = ".";
	  break;
	}
      ++s;
    }

  if (strcmp (s, ".") == 0)
    return 0;

  if (s[0] == '.' && s[1] == '/')
    s += 2;

  while (*s != '\0')
    {
      char *endp = strchrnul (s, '/');

      PRINTF ("looking for %.*s, level %d\n", (int) (endp - s), s, level);

      while (idx < nfiles && filesystem[idx].level >= level)
	{
	  if (filesystem[idx].level == level
	      && memcmp (s, filesystem[idx].name, endp - s) == 0
	      && filesystem[idx].name[endp - s] == '\0')
	    break;
	  ++idx;
	}

      if (idx == nfiles || filesystem[idx].level < level)
	{
	  errno = ENOENT;
	  return -1;
	}

      if (*endp == '\0')
	return idx + 1;

      if (filesystem[idx].type != DT_DIR
	  && (idx + 1 >= nfiles
	      || filesystem[idx].level >= filesystem[idx + 1].level))
	{
	  errno = ENOTDIR;
	  return -1;
	}

      ++idx;

      s = endp + 1;
      ++level;
    }

  errno = ENOENT;
  return -1;
}
