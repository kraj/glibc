/* Atomic operations.  X86 version.
   Copyright (C) 2018-2025 Free Software Foundation, Inc.
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

#ifndef _X86_ATOMIC_MACHINE_H
#define _X86_ATOMIC_MACHINE_H 1

#define USE_ATOMIC_COMPILER_BUILTINS	1

#define __HAVE_64B_ATOMICS		1

#define ATOMIC_EXCHANGE_USES_CAS	0

#define atomic_compare_and_exchange_val_acq(mem, newval, oldval)	\
  ({									\
    typeof (*mem) __oldval = (oldval);					\
    __atomic_compare_exchange_n (mem, (void *) &__oldval, newval, 0,	\
				 __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);	\
    __oldval;								\
  })

#define atomic_compare_and_exchange_bool_acq(mem, newval, oldval)	\
  ({									\
    typeof (*mem) __oldval = (oldval);					\
    !__atomic_compare_exchange_n (mem, (void *) &__oldval, newval, 0,	\
				  __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);	\
  })

#define atomic_exchange_acq(mem, newvalue) \
  __atomic_exchange_n (mem, newvalue, __ATOMIC_ACQUIRE)

/* ??? Remove when catomic_exchange_and_add
   fallback uses __atomic_fetch_add.  */
#define catomic_exchange_and_add(mem, value) \
  __atomic_fetch_add (mem, value, __ATOMIC_ACQUIRE)

#define atomic_full_barrier() __sync_synchronize ()
#define atomic_read_barrier() __asm ("" ::: "memory")
#define atomic_write_barrier() __asm ("" ::: "memory")

#define atomic_spin_nop() __asm ("pause")

#endif /* atomic-machine.h */
