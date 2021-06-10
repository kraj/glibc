/* TCB deallocation for NPTL.
   Copyright (C) 2002-2021 Free Software Foundation, Inc.
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

#include <nptl-stack.h>
#include <pthreadP.h>
#include <stdlib.h>

void
__nptl_free_tcb (struct pthread *pd)
{
   /* Free TPP data.  */
   if (pd->tpp != NULL)
     {
	struct priority_protection_data *tpp = pd->tpp;

	pd->tpp = NULL;
        free (tpp);
     }

  /* Queue the stack memory block for reuse and exit the process.  The kernel
     will signal via writing to the address of pthread 'joinstate' member
     (due CLONE_CHILD_CLEARTID) when the child exits.  */
  __nptl_deallocate_stack (pd);
}
libc_hidden_def (__nptl_free_tcb)
