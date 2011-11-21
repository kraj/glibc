/* Definition for thread-local data handling.  nptl/x32 version.
   Copyright (C) 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _X32_TLS_H
#define _X32_TLS_H	1

#include <sysdeps/x86_64/tls.h>

#ifndef __ASSEMBLER__

# undef THREAD_SELF
# define THREAD_SELF \
  ({ struct pthread *__self;						      \
     asm ("movl %%fs:%c1,%k0" : "=r" (__self)				      \
	  : "i" (offsetof (struct pthread, header.self)));	 	      \
     __self;})

# undef CALL_THREAD_FCT
# define CALL_THREAD_FCT(descr) \
  ({ void *__res;							      \
     asm volatile ("movl %%fs:%P2, %%edi\n\t"				      \
		   "movl %%fs:%P1, %k0\n\t"				      \
		   "callq *%q0"						      \
		   : "=a" (__res)					      \
		   : "i" (offsetof (struct pthread, start_routine)),	      \
		     "i" (offsetof (struct pthread, arg))		      \
		   : "di", "si", "cx", "dx", "r8", "r9", "r10", "r11",	      \
		     "memory", "cc");					      \
     __res; })

#endif /* __ASSEMBLER__ */

#endif	/* x32/tls.h */
