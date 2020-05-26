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

/* For RTLD_PRIVATE_ERRNO.  */
#include <dl-sysdep.h>
#ifndef __ASSEMBLER__
#include <tcbhead.h>
#endif

/* For Linux we can use the system call table in the header file
        /usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)   __NR_##syscall_name

#ifdef __ASSEMBLER__

#undef SYSCALL_ERROR_LABEL
#define SYSCALL_ERROR_LABEL __local_syscall_error

#undef PSEUDO
#define PSEUDO(name, syscall_name, args) \
  ENTRY (name)                           \
    DO_CALL (syscall_name, args)         \
    bne r7, zero, SYSCALL_ERROR_LABEL;   \

#undef PSEUDO_END
#define PSEUDO_END(name) \
  SYSCALL_ERROR_HANDLER  \
  END (name)

#undef PSEUDO_NOERRNO
#define PSEUDO_NOERRNO(name, syscall_name, args) \
  ENTRY (name)                                   \
    DO_CALL (syscall_name, args)

#undef PSEUDO_END_NOERRNO
#define PSEUDO_END_NOERRNO(name) \
  END (name)

#undef ret_NOERRNO
#define ret_NOERRNO ret

#undef DO_CALL
#define DO_CALL(syscall_name, args) \
    DOARGS_##args                   \
    movi r2, SYS_ify(syscall_name);  \
    trap;

#if defined(__PIC__) || defined(PIC)

# if RTLD_PRIVATE_ERRNO

#  define SYSCALL_ERROR_HANDLER			\
  SYSCALL_ERROR_LABEL:				\
  nextpc r3;					\
1:						\
  movhi r8, %hiadj(rtld_errno - 1b);		\
  addi r8, r8, %lo(rtld_errno - 1b);		\
  add r3, r3, r8;				\
  stw r2, 0(r3);				\
  movi r2, -1;					\
  ret;

# else

#  if IS_IN (libc)
#   define SYSCALL_ERROR_ERRNO __libc_errno
#  else
#   define SYSCALL_ERROR_ERRNO errno
#  endif
#  define SYSCALL_ERROR_HANDLER			\
  SYSCALL_ERROR_LABEL:				\
  nextpc r3;					\
1:						\
  movhi r8, %hiadj(_gp_got - 1b);		\
  addi r8, r8, %lo(_gp_got - 1b);		\
  add r3, r3, r8;				\
  ldw r3, %tls_ie(SYSCALL_ERROR_ERRNO)(r3);	\
  add r3, r23, r3;				\
  stw r2, 0(r3);				\
  movi r2, -1;					\
  ret;

# endif

#else

/* We can use a single error handler in the static library.  */
#define SYSCALL_ERROR_HANDLER			\
  SYSCALL_ERROR_LABEL:				\
  jmpi __syscall_error;

#endif

#define DOARGS_0 /* nothing */
#define DOARGS_1 /* nothing */
#define DOARGS_2 /* nothing */
#define DOARGS_3 /* nothing */
#define DOARGS_4 /* nothing */
#define DOARGS_5 ldw r8, 0(sp);
#define DOARGS_6 ldw r9, 4(sp); ldw r8, 0(sp);

/* The function has to return the error code.  */
#undef  PSEUDO_ERRVAL
#define PSEUDO_ERRVAL(name, syscall_name, args) \
  ENTRY (name)                                  \
    DO_CALL (syscall_name, args)

#undef  PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(name) \
  END (name)

#define ret_ERRVAL ret

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
