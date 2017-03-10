/* libresolv interfaces for internal use across glibc.
   Copyright (C) 2016-2017 Free Software Foundation, Inc.
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

#ifndef _RESOLV_INTERNAL_H
#define _RESOLV_INTERNAL_H 1

#include <resolv.h>
#include <stdbool.h>

/* Internal version of RES_USE_INET6 which does not trigger a
   deprecation warning.  */
#define DEPRECATED_RES_USE_INET6 0x00002000

static inline bool
res_use_inet6 (void)
{
  return _res.options & DEPRECATED_RES_USE_INET6;
}

struct out_buffer;

/* Convert the expanded domain name at SRC from wire format to text
   format.  Use storage in *DST.  Return a pointer to data in *DST, or
   NULL on error (and sets errno to EMSGSIZE).  */
char *__ns_name_ntop_buffer (struct out_buffer *, const u_char *)
  __THROW internal_function;
libresolv_hidden_proto (__ns_name_ntop_buffer)

int __ns_name_unpack (const u_char *, const u_char *,
                      const u_char *, u_char *, size_t) __THROW;

#endif  /* _RESOLV_INTERNAL_H */
