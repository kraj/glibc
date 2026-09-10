/* Copyright (C) 2001-2026 Free Software Foundation, Inc.
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

#include <errno.h>
#include <sysdep.h>
#include <pointer_guard.h>
#include <setjmp.h>
#include <bits/setjmp.h>
#include <stdlib.h>
#include <unistd.h>
#include <stap-probe.h>

/* Jump to the position specified by ENV, causing the
   setjmp call there to return VAL, or 1 if VAL is 0.  */
void
__longjmp (__jmp_buf env, int val)
{
  uintptr_t ret_addr = env->__gregs[8];
  uintptr_t env_sp = env->__gregs[9];
#ifdef PTR_DEMANGLE
  PTR_DEMANGLE (env_sp);
  PTR_DEMANGLE (ret_addr);
#endif
#ifdef CHECK_SP
  CHECK_SP (env_sp);
#endif
  register long int r2 __asm__ ("%r2") = val == 0 ? 1 : val;
  register uintptr_t r4 __asm__ ("%r4") = ret_addr;
  register uintptr_t r5 __asm__ ("%r5") = env_sp;
  /* Restore registers and jump back.  */
  __asm__ __volatile__ (
			/* longjmp probe expects longjmp first argument, second
			   argument and target address.  */
			LIBC_PROBE_ASM (longjmp, 8@%1 -4@%0 8@%2)

			/* restore fpregs  */
			"ld    %%f8,80(%1)\n\t"
			"ld    %%f9,88(%1)\n\t"
			"ld    %%f10,96(%1)\n\t"
			"ld    %%f11,104(%1)\n\t"
			"ld    %%f12,112(%1)\n\t"
			"ld    %%f13,120(%1)\n\t"
			"ld    %%f14,128(%1)\n\t"
			"ld    %%f15,136(%1)\n\t"

			/* restore gregs and return to jmp_buf target  */
			"lmg  %%r6,%%r13,0(%1)\n\t"
			"lgr  %%r15,%3\n\t"
			LIBC_PROBE_ASM (longjmp_target, 8@%1 -4@%0 8@%2)
			"br   %2"
			: : "r" (r2), "a" (env), "r" (r4), "r" (r5)
			);

  /* Avoid `volatile function does return' warnings.  */
  for (;;);
}
