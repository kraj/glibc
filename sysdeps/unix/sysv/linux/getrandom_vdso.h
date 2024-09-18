/* Linux getrandom vDSO support.
   Copyright (C) 2024 Free Software Foundation, Inc.
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

#ifndef _GETRANDOM_VDSO_H
#define _GETRANDOM_VDSO_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* Used to query the vDSO for the required mmap flags and the opaque
   per-thread state size  Defined by linux/random.h.  */
struct vgetrandom_opaque_params
{
  uint32_t size_of_opaque_state;
  uint32_t mmap_prot;
  uint32_t mmap_flags;
  uint32_t reserved[13];
};

#endif
