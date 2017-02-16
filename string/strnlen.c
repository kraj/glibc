/* Find the length of STRING, but scan at most MAXLEN characters.
   Copyright (C) 1991-2018 Free Software Foundation, Inc.
   Contributed by Jakub Jelinek <jakub@redhat.com>.

   Based on strlen written by Torbjorn Granlund (tege@sics.se),
   with help from Dan Sahlin (dan@sics.se);
   commentary by Jim Blandy (jimb@ai.mit.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <string.h>

/* Find the length of S, but scan at most MAXLEN characters.  If no
   '\0' terminator is found in that many characters, return MAXLEN.  */

#ifndef STRNLEN
# define STRNLEN __strnlen
#endif

size_t
STRNLEN (const char *str, size_t maxlen)
{
  const char *found = memchr (str, '\0', maxlen);
  return found ? found - str : maxlen;
}

weak_alias (__strnlen, strnlen)
libc_hidden_def (__strnlen)
libc_hidden_def (strnlen)
