/* Definition for thread control block.  Nios II version.
   Copyright (C) 2020 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _TCBHEAD_H
#define _TCBHEAD_H 1

#include <stdint.h>
#include <stddef.h>
#include <dl-dtv.h>

typedef struct
{
  dtv_t *dtv;
  uintptr_t pointer_guard;
  unsigned spare[6];
} tcbhead_t;

register struct pthread *__thread_self __asm__("r23");

/* The thread pointer (in hardware register r23) points to the end of
   the TCB + 0x7000, as for PowerPC and MIPS.  */
#define TLS_TCB_OFFSET 0x7000

static inline uintptr_t get_pointer_guard (void)
{
  tcbhead_t *tcbhead = (void *) __thread_self - TLS_TCB_OFFSET;
  return tcbhead[-1].pointer_guard;
}

static inline void set_pointer_guard (struct pthread *pd, uintptr_t guard)
{
  tcbhead_t *tcbhead = (void *) pd - TLS_TCB_OFFSET;
  tcbhead[-1].pointer_guard = guard;
}

#endif
