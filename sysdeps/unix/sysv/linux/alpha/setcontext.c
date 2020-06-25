/* Set the user context.  Linux/Alpha version.
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

#include <ucontext.h>
#include <sysdep.h>

int
__setcontext (const ucontext_t *ucp)
{
  /* In case the user fiddled it, copy the "official" signal mask
     from the ucontext_t into the sigcontext structure.  */
  ucontext_t *ucp_noconst = (ucontext_t *) ucp;
  ucp_noconst->uc_mcontext.sc_mask = *((long int *)&ucp_noconst->uc_sigmask);
  return inline_syscall (__NR_sigreturn, ucp_noconst);
}
weak_alias (__setcontext, setcontext)
