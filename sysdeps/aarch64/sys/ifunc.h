/* Definitions used by AArch64 indirect function resolvers.
   Copyright (C) 2019-2026 Free Software Foundation, Inc.
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

#ifndef _SYS_IFUNC_H
#define _SYS_IFUNC_H

#include <sys/cdefs.h>
#include <stdint.h>

/* A second argument is passed to the ifunc resolver.  */
#define _IFUNC_ARG_HWCAP	(1ULL << 62)

/* Maximum number of HWCAP elements that are currently supported.  */
#define _IFUNC_HWCAP_MAX	4

/* The prototype of a GNU indirect function resolver on AArch64 is

     ElfW(Addr) ifunc_resolver (uint64_t, const uint64_t *);

   The following prototype is also compatible:

     ElfW(Addr) ifunc_resolver (uint64_t, const __ifunc_arg_t *);

   The first argument might have the _IFUNC_ARG_HWCAP bit set and
   the remaining bits should match the AT_HWCAP settings.

   If the _IFUNC_ARG_HWCAP bit is set in the first argument, then
   the second argument is passed to the resolver function.  In
   this case, the second argument is a const pointer to a buffer
   that allows to access all available HWCAP elements.

   This buffer has its size in bytes at offset 0.  The HWCAP elements
   are available at offsets 8, 16, 24, 32... respectively for AT_HWCAP,
   AT_HWCAP2, AT_HWCAP3, AT_HWCAP4...  (these offsets are multiples of
   sizeof (uint64_t)).

   Indirect function resolvers must check availability of HWCAP
   elements at runtime before accessing them using the size of the
   buffer.  */

struct __ifunc_arg_t
{
  uint64_t _size;    /* Size of the struct, so it can grow.  */
  uint64_t _hwcap;
  uint64_t _hwcap2;  /* End of 1st published struct.  */
  uint64_t _hwcap3;
  uint64_t _hwcap4;  /* End of 2nd published struct.  */
};

typedef struct __ifunc_arg_t __ifunc_arg_t;

/* Constants for IDs of HWCAP elements to be used with the
   __ifunc_hwcap function below.  */
enum
{
  _IFUNC_ARG_AT_HWCAP = 1,
  _IFUNC_ARG_AT_HWCAP2 = 2,
  _IFUNC_ARG_AT_HWCAP3 = 3,
  _IFUNC_ARG_AT_HWCAP4 = 4,
};

/* A helper function to obtain HWCAP element by its ID from the
   parameters ARG0 and ARG1 passed to the ifunc resolver.  Note that
   ID 1 corresponds to AT_HWCAP, ID 2 corresponds to AT_HWCAP2, etc.
   If there is no element available for the requested ID then 0 is
   returned.  If ID doesn't much any supported AT_HWCAP{,2,...} value,
   then 0 is also returned.  */
static __inline uint64_t __attribute__ ((unused, always_inline))
__ifunc_hwcap (uint64_t __id,
	       uint64_t __arg0, const uint64_t *__arg1)
{
  if (__glibc_likely (__arg0 & _IFUNC_ARG_HWCAP))
    {
      const uint64_t size = __arg1[0];
      const uint64_t offset = __id * sizeof (uint64_t);
      return offset < size && __id > 0 ? __arg1[__id] : 0;
    }
  return __id == 1 ? __arg0 : 0;
}

#endif
