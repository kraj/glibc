/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
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

#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <string-fzb.h>
#include <string-fzi.h>
#include <string-extbyte.h>
#include <memcopy.h>

#undef strcpy

#ifndef STRCPY
# define STRCPY strcpy
#endif

/* Copy SRC to DEST.  */
char *
STRCPY (char *dest, const char *src)
{
  char *dst = dest;
  const op_t *xs;
  op_t *xd;
  op_t ws;

#if _STRING_ARCH_unaligned
  /* For architectures which supports unaligned memory operations, it first
     aligns the source pointer, reads op_t bytes at time until a zero is
     found, and writes unaligned to destination.  */
  uintptr_t n = -(uintptr_t) src % sizeof (op_t);
  for (uintptr_t i = 0; i < n; ++i)
    {
      unsigned c = *src++;
      *dst++ = c;
      if (c == '\0')
	return dest;
    }
  xs = (const op_t *) src;
  ws = *xs++;
  xd = (op_t *) dst;
  while (!has_zero (ws))
    {
      *xd++ = ws;
      ws = *xs++;
    }
#else
  /* For architectures which only supports aligned accesses, it first align
     the destination pointer.  */
  uintptr_t n = -(uintptr_t) dst % sizeof (op_t);
  for (uintptr_t i = 0; i < n; ++i)
    {
      unsigned c = *src++;
      *dst++ = c;
      if (c == '\0')
	return dest;
    }
  xd = (op_t *) dst;

  /* Destination is aligned to op_t while source might be not.  */
  uintptr_t ofs = (uintptr_t) src % sizeof (op_t);
  if (ofs == 0)
    {
      /* Aligned loop.  If a zero is found, exit to copy the remaining
	 bytes.  */
      xs = (const op_t *) src;

      ws = *xs++;
      while (!has_zero (ws))
	{
	  *xd++ = ws;
	  ws = *xs++;
	}
    }
  else
    {
      /* Unaligned loop: align the source pointer and mask off the
	 undesirable bytes which is not part of the string.  */
      op_t wsa, wsb;
      uintptr_t sh_1, sh_2;

      xs = (const op_t *)(src - ofs);
      wsa = *xs++;
      sh_1 = ofs * CHAR_BIT;
      sh_2 = sizeof(op_t) * CHAR_BIT - sh_1;

      /* Align the first partial op_t from source, with 0xff for the rest
	 of the bytes so that we can also apply the has_zero test to see if we
         have already reached EOS.  If we have, then we can simply fall
         through to the final byte copies.  */
      ws = MERGE (wsa, sh_1, (op_t)-1, sh_2);
      if (!has_zero (ws))
	{
	  while (1)
	    {
	      wsb = *xs++;
	      ws = MERGE (wsa, sh_1, wsb, sh_2);
	      if (has_zero (wsb))
		break;
	      *xd++ = ws;
	      wsa = wsb;
	    }

	  /* WS may contain bytes that we not written yet in destination.
	     Write them down and merge with the op_t containing the EOS
	     byte. */
	  if (!has_zero (ws))
	    {
	      *xd++ = ws;
	      ws = MERGE (wsb, sh_1, ws, sh_2);
	    }
	}
    }
#endif

  /* Just copy the final bytes from op_t.  */
  dst = (char *) xd;
  uintptr_t fz = index_first_zero (ws);
  for (uintptr_t i = 0; i < fz + 1; i++)
    *dst++ = extractbyte (ws, i);

  return dest;
}
libc_hidden_builtin_def (strcpy)
