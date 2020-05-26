/* Copyright (C) 2005-2020 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#ifndef _LINUX_AARCH64_SYSDEP_H
#define _LINUX_AARCH64_SYSDEP_H 1

#include <sysdeps/unix/sysdep.h>
#include <sysdeps/aarch64/sysdep.h>
#include <sysdeps/unix/sysv/linux/generic/sysdep.h>

/* Defines RTLD_PRIVATE_ERRNO and USE_DL_SYSINFO.  */
#include <dl-sysdep.h>

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <errno.h>
#endif

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)	(__NR_##syscall_name)

#ifdef __ASSEMBLER__

/* Linux uses a negative return value to indicate syscall errors,
   unlike most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be
   negative even if the call succeeded.  E.g., the `lseek' system call
   might return a large offset.  Therefore we must not anymore test
   for < 0, but test for a real error by making sure the value in R0
   is a real error number.  Linus said he will make sure the no syscall
   returns a value in -1 .. -4095 as a valid result so we can safely
   test with -4095.  */

# undef	PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name);								      \
    DO_CALL (syscall_name, args);					      \
    cmn x0, #4095;							      \
    b.cs .Lsyscall_error;

# undef	PSEUDO_END
# define PSEUDO_END(name)						      \
  SYSCALL_ERROR_HANDLER							      \
  END (name)

# undef	PSEUDO_NOERRNO
# define PSEUDO_NOERRNO(name, syscall_name, args)			      \
  .text;								      \
  ENTRY (name);								      \
    DO_CALL (syscall_name, args);

# undef	PSEUDO_END_NOERRNO
# define PSEUDO_END_NOERRNO(name)					      \
  END (name)

# define ret_NOERRNO ret

/* The function has to return the error code.  */
# undef	PSEUDO_ERRVAL
# define PSEUDO_ERRVAL(name, syscall_name, args) \
  .text;								      \
  ENTRY (name)								      \
    DO_CALL (syscall_name, args);					      \
    neg x0, x0

# undef	PSEUDO_END_ERRVAL
# define PSEUDO_END_ERRVAL(name) \
  END (name)

# define ret_ERRVAL ret

# if !IS_IN (libc)
#  define SYSCALL_ERROR  .Lsyscall_error
#  if RTLD_PRIVATE_ERRNO
#   define SYSCALL_ERROR_HANDLER				\
.Lsyscall_error:						\
	adrp	x1, C_SYMBOL_NAME(rtld_errno);			\
	neg     w0, w0;						\
	str     w0, [x1, :lo12:C_SYMBOL_NAME(rtld_errno)];	\
	mov	x0, -1;						\
	RET;
#  else

#   define SYSCALL_ERROR_HANDLER				\
.Lsyscall_error:						\
	adrp	x1, :gottprel:errno;				\
	neg	w2, w0;						\
	ldr	PTR_REG(1), [x1, :gottprel_lo12:errno];		\
	mrs	x3, tpidr_el0;					\
	mov	x0, -1;						\
	str	w2, [x1, x3];					\
	RET;
#  endif
# else
#  define SYSCALL_ERROR __syscall_error
#  define SYSCALL_ERROR_HANDLER                                 \
.Lsyscall_error:                                                \
	b	__syscall_error;
# endif

/* Linux takes system call args in registers:
	syscall number	x8
	arg 1		x0
	arg 2		x1
	arg 3		x2
	arg 4		x3
	arg 5		x4
	arg 6		x5
	arg 7		x6

   The compiler is going to form a call by coming here, through PSEUDO, with
   arguments
	syscall number	in the DO_CALL macro
	arg 1		x0
	arg 2		x1
	arg 3		x2
	arg 4		x3
	arg 5		x4
	arg 6		x5
	arg 7		x6

*/

# undef	DO_CALL
# define DO_CALL(syscall_name, args)		\
    mov x8, SYS_ify (syscall_name);		\
    svc 0

#else /* not __ASSEMBLER__ */

# ifdef __LP64__
#  define VDSO_NAME  "LINUX_2.6.39"
#  define VDSO_HASH  123718537
# else
#  define VDSO_NAME  "LINUX_4.9"
#  define VDSO_HASH  61765625
# endif

/* List of system calls which are supported as vsyscalls.  */
# define HAVE_CLOCK_GETRES64_VSYSCALL	"__kernel_clock_getres"
# define HAVE_CLOCK_GETTIME64_VSYSCALL	"__kernel_clock_gettime"
# define HAVE_GETTIMEOFDAY_VSYSCALL	"__kernel_gettimeofday"

/* Previously AArch64 used the generic version without the libc_hidden_def
   which lead in a non existent __send symbol in libc.so.  */
# undef HAVE_INTERNAL_SEND_SYMBOL

# define SINGLE_THREAD_BY_GLOBAL		1

static inline long int
__internal_syscall0 (long int name)
{
  register long int x8 asm ("x8") = name;
  register long int x0 asm ("x0");
  asm volatile ("svc 0"
		: "=r" (x0)
		: "r" (x8)
		: "memory");
  return x0;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  register long int x8 asm ("x8") = name;
  register long int x0 asm ("x0") = arg1;
  asm volatile ("svc 0"
		: "=r" (x0)
		: "r" (x8), "r" (x0)
		: "memory");
  return x0;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  register long int x8 asm ("x8") = name;
  register long int x0 asm ("x0") = arg1;
  register long int x1 asm ("x1") = arg2;
  asm volatile ("svc 0"
		: "=r" (x0)
		: "r" (x8), "r" (x0), "r" (x1)
		: "memory");
  return x0;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register long int x8 asm ("x8") = name;
  register long int x0 asm ("x0") = arg1;
  register long int x1 asm ("x1") = arg2;
  register long int x2 asm ("x2") = arg3;
  asm volatile ("svc 0"
		: "=r" (x0)
		: "r" (x8), "r" (x0), "r" (x1), "r" (x2)
		: "memory");
  return x0;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  register long int x8 asm ("x8") = name;
  register long int x0 asm ("x0") = arg1;
  register long int x1 asm ("x1") = arg2;
  register long int x2 asm ("x2") = arg3;
  register long int x3 asm ("x3") = arg4;
  asm volatile ("svc 0"
		: "=r" (x0)
		: "r" (x8), "r" (x0), "r" (x1), "r" (x2), "r" (x3)
		: "memory");
  return x0;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  register long int x8 asm ("x8") = name;
  register long int x0 asm ("x0") = arg1;
  register long int x1 asm ("x1") = arg2;
  register long int x2 asm ("x2") = arg3;
  register long int x3 asm ("x3") = arg4;
  register long int x4 asm ("x4") = arg5;
  asm volatile ("svc 0"
		: "=r" (x0)
		: "r" (x8), "r" (x0), "r" (x1), "r" (x2), "r" (x3), "r" (x4)
		: "memory");
  return x0;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  register long int x8 asm ("x8") = name;
  register long int x0 asm ("x0") = arg1;
  register long int x1 asm ("x1") = arg2;
  register long int x2 asm ("x2") = arg3;
  register long int x3 asm ("x3") = arg4;
  register long int x4 asm ("x4") = arg5;
  register long int x5 asm ("x5") = arg6;
  asm volatile ("svc 0"
		: "=r" (x0)
		: "r" (x8), "r" (x0), "r" (x1), "r" (x2), "r" (x3), "r" (x4),
		  "r" (x5)
		: "memory");
  return x0;
}

#endif	/* __ASSEMBLER__ */

/* Pointer mangling is supported for AArch64.  */
#if (IS_IN (rtld) \
     || (!defined SHARED && (IS_IN (libc) \
			     || IS_IN (libpthread))))
# ifdef __ASSEMBLER__
/* Note, dst, src, guard, and tmp are all register numbers rather than
   register names so they will work with both ILP32 and LP64. */
#  define PTR_MANGLE(dst, src, guard, tmp)                                \
  LDST_PCREL (ldr, guard, tmp, C_SYMBOL_NAME(__pointer_chk_guard_local)); \
  PTR_MANGLE2 (dst, src, guard)
/* Use PTR_MANGLE2 for efficiency if guard is already loaded.  */
#  define PTR_MANGLE2(dst, src, guard)\
  eor x##dst, x##src, x##guard
#  define PTR_DEMANGLE(dst, src, guard, tmp)\
  PTR_MANGLE (dst, src, guard, tmp)
#  define PTR_DEMANGLE2(dst, src, guard)\
  PTR_MANGLE2 (dst, src, guard)
# else
extern uintptr_t __pointer_chk_guard_local attribute_relro attribute_hidden;
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ __pointer_chk_guard_local)
#  define PTR_DEMANGLE(var)     PTR_MANGLE (var)
# endif
#else
# ifdef __ASSEMBLER__
/* Note, dst, src, guard, and tmp are all register numbers rather than
   register names so they will work with both ILP32 and LP64. */
#  define PTR_MANGLE(dst, src, guard, tmp)                             \
  LDST_GLOBAL (ldr, guard, tmp, C_SYMBOL_NAME(__pointer_chk_guard));   \
  PTR_MANGLE2 (dst, src, guard)
/* Use PTR_MANGLE2 for efficiency if guard is already loaded.  */
#  define PTR_MANGLE2(dst, src, guard)\
  eor x##dst, x##src, x##guard
#  define PTR_DEMANGLE(dst, src, guard, tmp)\
  PTR_MANGLE (dst, src, guard, tmp)
#  define PTR_DEMANGLE2(dst, src, guard)\
  PTR_MANGLE2 (dst, src, guard)
# else
extern uintptr_t __pointer_chk_guard attribute_relro;
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ __pointer_chk_guard)
#  define PTR_DEMANGLE(var) PTR_MANGLE (var)
# endif
#endif

#endif /* linux/aarch64/sysdep.h */
