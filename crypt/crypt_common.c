/*
 * crypt: crypt(3) implementation
 *
 * Copyright (C) 1991-2014 Free Software Foundation, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; see the file COPYING.LIB.  If not,
 * see <http://www.gnu.org/licenses/>.
 *
 * General Support routines
 *
 */

#include "crypt-private.h"

/* Table with characters for base64 transformation.  */
static const char b64t[64] =
"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void
__b64_from_24bit (char **cp, int *buflen,
		  unsigned int b2, unsigned int b1, unsigned int b0,
		  int n)
{
  unsigned int w = (b2 << 16) | (b1 << 8) | b0;
  while (n-- > 0 && (*buflen) > 0)
    {
      *(*cp)++ = b64t[w & 0x3f];
      --(*buflen);
      w >>= 6;
    }
}
