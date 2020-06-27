/* Assembler macros for Nios II.
   Copyright (C) 2000-2020 Free Software Foundation, Inc.
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

#ifndef _LINUX_NIOS2_SYSDEP_H
#define _LINUX_NIOS2_SYSDEP_H 1

#include <sysdeps/unix/sysdep.h>
#include <sysdeps/nios2/sysdep.h>
#include <sysdeps/unix/sysv/linux/generic/sysdep.h>

#ifndef __ASSEMBLER__
#include <tcbhead.h>
#endif

#ifdef __ASSEMBLER__

#define SYSCALL_ERROR_LABEL __local_syscall_error

#if defined(__PIC__) || defined(PIC)
# define SYSCALL_ERROR_HANDLER			\
  SYSCALL_ERROR_LABEL:				\
  nextpc r3;					\
1:						\
  movhi r8, %hiadj(_gp_got - 1b);		\
  addi r8, r8, %lo(_gp_got - 1b);		\
  add r3, r3, r8;				\
  ldw r3, %tls_ie(__libc_errno)(r3);		\
  add r3, r23, r3;				\
  stw r2, 0(r3);				\
  movi r2, -1;					\
  ret;

#else

/* We can use a single error handler in the static library.  */
#define SYSCALL_ERROR_HANDLER			\
  SYSCALL_ERROR_LABEL:				\
  jmpi __syscall_error;
#endif /* __PIC__ || PIC  */

#else /* __ASSEMBLER__ */

/* Previously Nios2 used the generic version without the libc_hidden_def
   which lead in a non existent __send symbol in libc.so.  */
# undef HAVE_INTERNAL_SEND_SYMBOL

static inline long int
__internal_syscall0 (long int name)
{
  register int r2 asm ("r2") = name;
  register int err asm ("r7");
  asm volatile ("trap"
		: "+r" (r2), "=r" (err)
                :
                : "memory");
  return err != 0 ? -r2 : r2;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  register int r2 asm ("r2") = name;
  register int r4 asm ("r4") = arg1;
  register int r7 asm ("r7");
  asm volatile ("trap"
		: "+r" (r2), "=r" (r7)
                : "r" (r4)
                : "memory");
  return r7 != 0 ? -r2 : r2;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  register int r2 asm ("r2") = name;
  register int r4 asm ("r4") = arg1;
  register int r5 asm ("r5") = arg2;
  register int r7 asm ("r7");
  asm volatile ("trap"
		: "+r" (r2), "=r" (r7)
                : "r" (r4), "r" (r5)
                : "memory");
  return r7 != 0 ? -r2 : r2;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register int r2 asm ("r2") = name;
  register int r4 asm ("r4") = arg1;
  register int r5 asm ("r5") = arg2;
  register int r6 asm ("r6") = arg3;
  register int r7 asm ("r7");
  asm volatile ("trap"
		: "+r" (r2), "=r" (r7)
                : "r" (r4), "r" (r5), "r" (r6)
                : "memory");
  return r7 != 0 ? -r2 : r2;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  register int r2 asm ("r2") = name;
  register int r4 asm ("r4") = arg1;
  register int r5 asm ("r5") = arg2;
  register int r6 asm ("r6") = arg3;
  register int r7 asm ("r7") = arg4;
  asm volatile ("trap"
		: "+r" (r2), "+r" (r7)
                : "r" (r4), "r" (r5), "r" (r6)
                : "memory");
  return r7 != 0 ? -r2 : r2;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  register int r2 asm ("r2") = name;
  register int r4 asm ("r4") = arg1;
  register int r5 asm ("r5") = arg2;
  register int r6 asm ("r6") = arg3;
  register int r7 asm ("r7") = arg4;
  register int r8 asm ("r8") = arg5;
  asm volatile ("trap"
		: "+r" (r2), "+r" (r7)
                : "r" (r4), "r" (r5), "r" (r6), "r" (r8)
                : "memory");
  return r7 != 0 ? -r2 : r2;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  register int r2 asm ("r2") = name;
  register int r4 asm ("r4") = arg1;
  register int r5 asm ("r5") = arg2;
  register int r6 asm ("r6") = arg3;
  register int r7 asm ("r7") = arg4;
  register int r8 asm ("r8") = arg5;
  register int r9 asm ("r9") = arg6;
  asm volatile ("trap"
		: "+r" (r2), "+r" (r7)
                : "r" (r4), "r" (r5), "r" (r6), "r" (r8), "r" (r9)
                : "memory");
  return r7 != 0 ? -r2 : r2;
}

#undef HAVE_INTERNAL_BRK_ADDR_SYMBOL
#define HAVE_INTERNAL_BRK_ADDR_SYMBOL 1

#endif /* __ASSEMBLER__ */

/* Pointer mangling support.  */
#if IS_IN (rtld)
/* We cannot use the thread descriptor because in ld.so we use setjmp
   earlier than the descriptor is initialized.  */
#else
# include <tcb-offsets.h>
# ifdef __ASSEMBLER__
#  define PTR_MANGLE_GUARD(guard) ldw guard, POINTER_GUARD(r23)
#  define PTR_MANGLE(dst, src, guard) xor dst, src, guard
#  define PTR_DEMANGLE(dst, src, guard) PTR_MANGLE (dst, src, guard)
# else
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ get_pointer_guard ())
#  define PTR_DEMANGLE(var)	PTR_MANGLE (var)
# endif
#endif


#endif /* linux/nios2/sysdep.h */
