/* RISC-V instruction cache flushing interface
   Copyright (C) 2017-2025 Free Software Foundation, Inc.

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

#ifndef _SYS_CACHECTL_H
#define _SYS_CACHECTL_H 1

#include <features.h>

__BEGIN_DECLS

extern int __riscv_flush_icache (void *__start, void *__end,
				 unsigned long int __flags);

__END_DECLS

#endif /* sys/cachectl.h */
