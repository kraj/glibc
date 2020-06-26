/* Copyright (C) 1992-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>, August 1995.

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

#ifndef _LINUX_ALPHA_SYSDEP_H
#define _LINUX_ALPHA_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/unix/sysdep.h>
#include <sysdeps/alpha/sysdep.h>

#ifndef __ASSEMBLER__

/* The normal Alpha calling convention sign-extends 32-bit quantties
   no matter what the "real" sign of the 32-bit type.  We want to
   preserve that when filling in values for the kernel.  */
#define syscall_promote(arg) \
  (sizeof (arg) == 4 ? (long int)(int)(long int)(arg) : (long int)(arg))

#define internal_syscall_clobbers				\
	"$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8",	\
	"$22", "$23", "$24", "$25", "$27", "$28", "memory"

static inline long int
__internal_syscall0 (long int name)
{
  register long int sc_19 asm ("$19");
  register long int sc_0 = name;
  asm volatile ("callsys # %0 %1 <= %2"
		: "+v" (sc_0), "=r" (sc_19)
		:
		: internal_syscall_clobbers,
		"$16", "$17", "$18", "$20", "$21");
  return sc_19 != 0 ? -sc_0 : sc_0;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  register long int sc_16 asm ("$16") = arg1;
  register long int sc_19 asm ("$19");
  register long int sc_0 = name;
  asm volatile ("callsys # %0 %1 <= %2 %3"
		: "+v" (sc_0), "=r" (sc_19), "+r" (sc_16)
		:
		: internal_syscall_clobbers,
		  "$17", "$18", "$20", "$21");
  return sc_19 != 0 ? -sc_0 : sc_0;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  register long int sc_16 asm ("$16") = arg1;
  register long int sc_17 asm ("$17") = arg2;
  register long int sc_19 asm ("$19");
  register long int sc_0 = name;
  asm volatile ("callsys # %0 %1 <= %2 %3 %4"
		: "+v" (sc_0), "=r" (sc_19), "+r" (sc_16), "+r" (sc_17)
		:
		: internal_syscall_clobbers,
		  "$18", "$20", "$21");
  return sc_19 != 0 ? -sc_0 : sc_0;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register long int sc_16 asm ("$16") = arg1;
  register long int sc_17 asm ("$17") = arg2;
  register long int sc_18 asm ("$18") = arg3;
  register long int sc_19 asm ("$19");
  register long int sc_0 = name;
  asm volatile ("callsys # %0 %1 <= %2 %3 %4 %5"
		: "+v" (sc_0), "=r" (sc_19), "+r" (sc_16), "+r" (sc_17),
		  "+r" (sc_18)
		:
		: internal_syscall_clobbers,
		  "$20", "$21");
  return sc_19 != 0 ? -sc_0 : sc_0;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  register long int sc_16 asm ("$16") = arg1;
  register long int sc_17 asm ("$17") = arg2;
  register long int sc_18 asm ("$18") = arg3;
  register long int sc_19 asm ("$19") = arg4;
  register long int sc_0 = name;
  asm volatile ("callsys # %0 %1 <= %2 %3 %4 %5 %6"
		: "+v" (sc_0), "+r" (sc_19), "+r" (sc_16), "+r" (sc_17),
		  "+r" (sc_18)
		:
		: internal_syscall_clobbers,
		  "$20", "$21");
  return sc_19 != 0 ? -sc_0 : sc_0;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  register long int sc_16 asm ("$16") = arg1;
  register long int sc_17 asm ("$17") = arg2;
  register long int sc_18 asm ("$18") = arg3;
  register long int sc_19 asm ("$19") = arg4;
  register long int sc_20 asm ("$20") = arg5;
  register long int sc_0 = name;
  asm volatile ("callsys # %0 %1 <= %2 %3 %4 %5 %6 %7"
		: "+v" (sc_0), "+r" (sc_19), "+r" (sc_16), "+r" (sc_17),
		  "+r" (sc_18), "+r" (sc_20)
		:
		: internal_syscall_clobbers,
		  "$21");
  return sc_19 != 0 ? -sc_0 : sc_0;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  register long int sc_16 asm ("$16") = arg1;
  register long int sc_17 asm ("$17") = arg2;
  register long int sc_18 asm ("$18") = arg3;
  register long int sc_19 asm ("$19") = arg4;
  register long int sc_20 asm ("$20") = arg5;
  register long int sc_21 asm ("$21") = arg6;
  register long int sc_0 = name;
  asm volatile ("callsys # %0 %1 <= %2 %3 %4 %5 %6 %7 %8"
		: "+v" (sc_0), "+r" (sc_19), "+r" (sc_16), "+r" (sc_17),
		  "+r" (sc_18), "+r" (sc_20), "+r" (sc_21)
		:
		: internal_syscall_clobbers);
  return sc_19 != 0 ? -sc_0 : sc_0;
}

struct pair_t
{
  long int sc_0;
  long int sc_20;
};

static inline struct pair_t
__internal_syscall_pair (long int name)
{
  register long int sc_19 asm ("$19");
  register long int sc_20 asm ("$20");
  register long int sc_0 = name;
  asm volatile ("callsys # %0 %1 <= %2"
		: "+v" (sc_0), "=r" (sc_19), "=r" (sc_20)
		:
		: internal_syscall_clobbers,
		"$16", "$17", "$18", "$21");
  return (struct pair_t) { sc_0, sc_20 };
}

#endif /* ASSEMBLER */

/* Pointer mangling support.  Note that tls access is slow enough that
   we don't deoptimize things by placing the pointer check value there.  */

#ifdef __ASSEMBLER__
# if IS_IN (rtld)
#  define PTR_MANGLE(dst, src, tmp)				\
	ldah	tmp, __pointer_chk_guard_local($29) !gprelhigh;	\
	ldq	tmp, __pointer_chk_guard_local(tmp) !gprellow;	\
	xor	src, tmp, dst
#  define PTR_MANGLE2(dst, src, tmp)				\
	xor	src, tmp, dst
# elif defined SHARED
#  define PTR_MANGLE(dst, src, tmp)		\
	ldq	tmp, __pointer_chk_guard;	\
	xor	src, tmp, dst
# else
#  define PTR_MANGLE(dst, src, tmp)		\
	ldq	tmp, __pointer_chk_guard_local;	\
	xor	src, tmp, dst
# endif
# define PTR_MANGLE2(dst, src, tmp)		\
	xor	src, tmp, dst
# define PTR_DEMANGLE(dst, tmp)   PTR_MANGLE(dst, dst, tmp)
# define PTR_DEMANGLE2(dst, tmp)  PTR_MANGLE2(dst, dst, tmp)
#else
# include <stdint.h>
# if (IS_IN (rtld) \
      || (!defined SHARED && (IS_IN (libc) \
			      || IS_IN (libpthread))))
extern uintptr_t __pointer_chk_guard_local attribute_relro attribute_hidden;
#  define PTR_MANGLE(var) \
	(var) = (__typeof (var)) ((uintptr_t) (var) ^ __pointer_chk_guard_local)
# else
extern uintptr_t __pointer_chk_guard attribute_relro;
#  define PTR_MANGLE(var) \
	(var) = (__typeof(var)) ((uintptr_t) (var) ^ __pointer_chk_guard)
# endif
# define PTR_DEMANGLE(var)  PTR_MANGLE(var)
#endif /* ASSEMBLER */

#endif /* _LINUX_ALPHA_SYSDEP_H  */
