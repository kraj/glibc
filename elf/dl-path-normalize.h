/* In-place lexical path normalization for the dynamic loader.
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

#ifndef _DL_PATH_NORMALIZE_H
#define _DL_PATH_NORMALIZE_H

#include <stddef.h>
#include <string.h>

/* Lexically normalize the null-terminated PATH in place and return the
   length of the result (excluding the terminating NUL byte):

   - Runs of '/' are collapsed to a single '/'.

   - "." components are removed.

   - A ".." component removes the preceding component if there is one and it
     is not itself a preserved "..".  In an absolute path a surplus ".." at
     the root is dropped ("/../a" normalizes to "/a"), in a relative path
     leading ".." components are preserved ("../a" stays "../a", "a/../../b"
     normalizes to "../b").

   - The result has no trailing '/' except for the root path "/" itself
     ("/a/" normalizes to "/a").

   - The result is empty if and only if every component cancels or is removed
     ("", ".", "a/.." all normalize to "").

   The PATH is written in place, and the internal write cursor never runs
   ahead of the read cursor.  Only bytes within the strlen (PATH) + 1 storage
   are accessed.  */
static inline size_t
_dl_normalize_path (char *path)
{
  /* The root '/' of an absolute path is not removed.  */
  char *pstart = path + (path[0] == '/');
  const char *rnp = pstart;
  char *wnp = pstart;
  /* End of the prefix a ".." may not remove.  Either the root '/', or, for
     relative paths, the original start of the string extended by any
     preserved leading ".." components.  */
  char *limit = pstart;

  while (*rnp != '\0')
    {
      /* Collapse consecutive separators.  */
      if (*rnp == '/')
	{
	  ++rnp;
	  continue;
	}

      /* [RNP, REND) is the next input component.  */
      const char *rend = rnp;
      while (*rend != '\0' && *rend != '/')
	++rend;
      size_t clen = rend - rnp;

      /* Drop '.' component.  */
      if (clen == 1 && rnp[0] == '.')
	;
      else if (clen == 2 && rnp[0] == '.' && rnp[1] == '.')
	{
	  if (wnp > limit)
	    {
	      /* Remove the last component along with the '/' separating it
		 from its predecessor (the root '/' of an absolute path is
		 retained).  */
	      while (wnp > limit && wnp[-1] != '/')
		--wnp;
	      if (wnp > pstart)
		--wnp;
	    }
	  else if (pstart == path)
	    {
	      /* No component is left and the original path is relative:
		 keep the unresolvable ".." (it becomes part of the
		 preserved prefix).  */
	      if (wnp > pstart)
		*wnp++ = '/';
	      *wnp++ = '.';
	      *wnp++ = '.';
	      limit = wnp;
	    }
	  /* Otherwise the path is absolute and the surplus ".." at the
	     root is dropped ("/../a" normalizes to "/a").  */
	}
      else
	{
	  if (wnp > pstart)
	    *wnp++ = '/';
	  memmove (wnp, rnp, clen);
	  wnp += clen;
	}

      rnp = rend;
    }

  *wnp = '\0';
  return wnp - path;
}

#endif /* _DL_PATH_NORMALIZE_H */
