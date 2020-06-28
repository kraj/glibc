/* Copyright (C) 1991-2020 Free Software Foundation, Inc.
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

#include <sysdeps/generic/sysdep.h>
#include <single-thread.h>
#include <sys/syscall.h>
#define	HAVE_SYSCALLS

#define __SYSCALL_CONCAT_X(a,b)     a##b
#define __SYSCALL_CONCAT(a,b)       __SYSCALL_CONCAT_X (a, b)

#define __INTERNAL_SYSCALL_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __INTERNAL_SYSCALL_NARGS(...) \
  __INTERNAL_SYSCALL_NARGS_X (__VA_ARGS__,7,6,5,4,3,2,1,0,)
#define __INTERNAL_SYSCALL_DISP(b,...) \
  __SYSCALL_CONCAT (b,__INTERNAL_SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

#ifndef __ASSEMBLER__
# ifndef ARGIFY
#  define ARGIFY(__x) ((__syscall_arg_t) (__x))
typedef long int __syscall_arg_t;
# endif
#endif

#define __internal_syscall_0(name) 					\
  __internal_syscall0 (name)
#define __internal_syscall_1(name, a1) 					\
  __internal_syscall1 (name, ARGIFY (a1))
#define __internal_syscall_2(name, a1, a2) 				\
  __internal_syscall2 (name, ARGIFY (a1), ARGIFY (a2))
#define __internal_syscall_3(name, a1, a2, a3) 				\
  __internal_syscall3 (name, ARGIFY (a1), ARGIFY (a2), ARGIFY (a3))
#define __internal_syscall_4(name, a1, a2, a3, a4) 			\
  __internal_syscall4 (name, ARGIFY (a1), ARGIFY (a2), ARGIFY (a3),	\
		       ARGIFY (a4))
#define __internal_syscall_5(name, a1, a2, a3, a4, a5) 			\
  __internal_syscall5 (name, ARGIFY (a1), ARGIFY (a2), ARGIFY (a3),	\
		       ARGIFY (a4), ARGIFY (a5))
#define __internal_syscall_6(name, a1, a2, a3, a4, a5, a6) 		\
  __internal_syscall6 (name, ARGIFY (a1), ARGIFY (a2), ARGIFY (a3),	\
		       ARGIFY (a4), ARGIFY (a5), ARGIFY (a6))
#define __internal_syscall_7(name, a1, a2, a3, a4, a5, a6, a7) 		\
  __internal_syscall7 (name, ARGIFY (a1), ARGIFY (a2), ARGIFY (a3),	\
		       ARGIFY (a4), ARGIFY (a5), ARGIFY (a6), ARGIFY (a7))

#define internal_syscall(...)						\
  __INTERNAL_SYSCALL_DISP(__internal_syscall_,__VA_ARGS__)

#define inline_syscall(...)						\
  __syscall_ret (internal_syscall (__VA_ARGS__))

#define internal_syscall_cancel(...) 					\
  ({									\
    long int __sc_ret;							\
    int __sc_cancel_oldtype = -1;					\
    if (! SINGLE_THREAD_P)						\
      __sc_cancel_oldtype = LIBC_CANCEL_ASYNC ();			\
    __sc_ret = internal_syscall (__VA_ARGS__);				\
    if (__sc_cancel_oldtype != -1)					\
      LIBC_CANCEL_RESET (__sc_cancel_oldtype);				\
    __sc_ret;								\
  })

#define inline_syscall_cancel(...)					\
  __syscall_ret (internal_syscall_cancel(__VA_ARGS__))
