/* Definition for thread control block handling.  IA-64 version.
   Copyright (C) 2003-2020 Free Software Foundation, Inc.
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

#ifndef _TCBHEAD_H
#define _TCBHEAD_H

#include <stddef.h>
#include <dl-dtv.h>

typedef struct
{
  dtv_t *dtv;
  void *__private;
} tcbhead_t;

register struct pthread *__thread_self __asm__("r13");

# define TLS_MULTIPLE_THREADS_IN_TCB 1

#endif
