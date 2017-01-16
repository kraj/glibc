/* Architecture specific bits for cancellation handling.
   Copyright (C) 2023 Free Software Foundation, Inc.
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

#ifndef _NPTL_CANCELLATION_PC_CHECK
#define _NPTL_CANCELLATION_PC_CHECK 1

/* Check if the program counter (PC) from ucontext CTX is within the start and
   then end boundary from the __syscall_cancel_arch bridge.  Return TRUE if
   the PC is within the boundary, meaning the syscall does not have any side
   effects; or FALSE otherwise.  */
static bool
cancellation_pc_check (void *ctx)
{
  /* Both are defined in syscall_cancel.S for each architecture.  */
  extern const char __syscall_cancel_arch_start[1];
  extern const char __syscall_cancel_arch_end[1];

  uintptr_t sc_ip = ((struct sigcontext *) (ctx))->sc_ip;
  uintptr_t cr_iip = sc_ip & ~0x3ull;
  uintptr_t ri = sc_ip & 0x3ull;

  /* IA64 __syscall_cancel_arch issues the 'break 0x100000' on its own bundle,
     and __syscall_cancel_arch_end points to end of the previous bundle.
     To check if the syscall had any side-effects we need to check the slot
     number.  */
  if (cr_iip == (uintptr_t) __syscall_cancel_arch_end)
    return ri == 0;

  return cr_iip >= (uintptr_t) __syscall_cancel_arch_start
	 && cr_iip < (uintptr_t) __syscall_cancel_arch_end;
}

#endif
