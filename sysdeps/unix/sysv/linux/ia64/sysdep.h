/* Copyright (C) 1999-2020 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Jes Sorensen, <Jes.Sorensen@cern.ch>, April 1999.
   Based on code originally written by David Mosberger-Tang

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

#ifndef _LINUX_IA64_SYSDEP_H
#define _LINUX_IA64_SYSDEP_H 1

#include <sysdeps/unix/sysdep.h>
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/ia64/sysdep.h>
#include <dl-sysdep.h>
#include <asm/break.h>
#include <tcb-offsets.h>

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <tcbhead.h>
#include <errno.h>
#endif

/* As of GAS v2.4.90.0.7, including a ".align" directive inside a
   function will cause bad unwind info to be emitted (GAS doesn't know
   how to account for the padding introduced by the .align directive).
   Turning on this macro will work around this bug by introducing the
   necessary padding explicitly. */
#define GAS_ALIGN_BREAKS_UNWIND_INFO

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)	__NR_##syscall_name

#if defined USE_DL_SYSINFO \
	&& (IS_IN (libc) \
	    || IS_IN (libpthread) || IS_IN (librt))
# define IA64_USE_NEW_STUB
#else
# undef IA64_USE_NEW_STUB
#endif

#ifdef __ASSEMBLER__

#undef CALL_MCOUNT
#ifdef PROF
# define CALL_MCOUNT							\
	.data;								\
1:	data8 0;	/* XXX fixme: use .xdata8 once labels work */	\
	.previous;							\
	.prologue;							\
	.save ar.pfs, r40;						\
	alloc out0 = ar.pfs, 8, 0, 4, 0;				\
	mov out1 = gp;							\
	.save rp, out2;							\
	mov out2 = rp;							\
	.body;								\
	;;								\
	addl out3 = @ltoff(1b), gp;					\
	br.call.sptk.many rp = _mcount					\
	;;
#else
# define CALL_MCOUNT	/* Do nothing. */
#endif

/* Linux uses a negative return value to indicate syscall errors, unlike
   most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be negative
   even if the call succeeded.  E.g., the `lseek' system call might return
   a large offset.  Therefore we must not anymore test for < 0, but test
   for a real error by making sure the value in %d0 is a real error
   number.  Linus said he will make sure the no syscall returns a value
   in -1 .. -4095 as a valid result so we can savely test with -4095.  */

/* We don't want the label for the error handler to be visible in the symbol
   table when we define it here.  */
#undef SYSCALL_ERROR_LABEL
#define SYSCALL_ERROR_LABEL __syscall_error

#undef PSEUDO
#define	PSEUDO(name, syscall_name, args)	\
  ENTRY(name)					\
    DO_CALL (SYS_ify(syscall_name));		\
	cmp.eq p6,p0=-1,r10;			\
(p6)	br.cond.spnt.few __syscall_error;

#define DO_CALL_VIA_BREAK(num)			\
	mov r15=num;				\
	break __IA64_BREAK_SYSCALL

#ifdef IA64_USE_NEW_STUB
# ifdef SHARED
#  define DO_CALL(num)				\
	.prologue;				\
	adds r2 = SYSINFO_OFFSET, r13;;		\
	ld8 r2 = [r2];				\
	.save ar.pfs, r11;			\
	mov r11 = ar.pfs;;			\
	.body;					\
	mov r15 = num;				\
	mov b7 = r2;				\
	br.call.sptk.many b6 = b7;;		\
	.restore sp;				\
	mov ar.pfs = r11;			\
	.prologue;				\
	.body
# else /* !SHARED */
#  define DO_CALL(num)				\
	.prologue;				\
	mov r15 = num;				\
	movl r2 = _dl_sysinfo;;			\
	ld8 r2 = [r2];				\
	.save ar.pfs, r11;			\
	mov r11 = ar.pfs;;			\
	.body;					\
	mov b7 = r2;				\
	br.call.sptk.many b6 = b7;;		\
	.restore sp;				\
	mov ar.pfs = r11;			\
	.prologue;				\
	.body
# endif
#else
# define DO_CALL(num)				DO_CALL_VIA_BREAK(num)
#endif

#undef PSEUDO_END
#define PSEUDO_END(name)	.endp C_SYMBOL_NAME(name);

#undef PSEUDO_NOERRNO
#define	PSEUDO_NOERRNO(name, syscall_name, args)	\
  ENTRY(name)						\
    DO_CALL (SYS_ify(syscall_name));

#undef PSEUDO_END_NOERRNO
#define PSEUDO_END_NOERRNO(name)	.endp C_SYMBOL_NAME(name);

#undef PSEUDO_ERRVAL
#define	PSEUDO_ERRVAL(name, syscall_name, args)	\
  ENTRY(name)					\
    DO_CALL (SYS_ify(syscall_name));		\
	cmp.eq p6,p0=-1,r10;			\
(p6)	mov r10=r8;


#undef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(name)	.endp C_SYMBOL_NAME(name);

#undef END
#define END(name)						\
	.size	C_SYMBOL_NAME(name), . - C_SYMBOL_NAME(name) ;	\
	.endp	C_SYMBOL_NAME(name)

#define ret			br.ret.sptk.few b0
#define ret_NOERRNO		ret
#define ret_ERRVAL		ret

#else /* not __ASSEMBLER__ */

#define BREAK_INSN_1(num) "break " #num ";;\n\t"
#define BREAK_INSN(num) BREAK_INSN_1(num)

/* On IA-64 we have stacked registers for passing arguments.  The
   "out" registers end up being the called function's "in"
   registers.

   Also, since we have plenty of registers we have two return values
   from a syscall.  r10 is set to -1 on error, whilst r8 contains the
   (non-negative) errno on error or the return value on success.
 */

#define ASM_CLOBBERS_0	ASM_CLOBBERS_1, "out0"
#define ASM_CLOBBERS_1	ASM_CLOBBERS_2, "out1"
#define ASM_CLOBBERS_2	ASM_CLOBBERS_3, "out2"
#define ASM_CLOBBERS_3	ASM_CLOBBERS_4, "out3"
#define ASM_CLOBBERS_4	ASM_CLOBBERS_5, "out4"
#define ASM_CLOBBERS_5	ASM_CLOBBERS_6, "out5"
#define ASM_CLOBBERS_6_COMMON	, "out6", "out7",			\
  /* Non-stacked integer registers, minus r8, r10, r15.  */		\
  "r2", "r3", "r9", "r11", "r13", "r14", "r16", "r17", "r18",		\
  "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",	\
  "r28", "r29", "r30", "r31",						\
  /* Predicate registers.  */						\
  "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15",	\
  /* Non-rotating fp registers.  */					\
  "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",	\
  /* Branch registers.  */						\
  "b6"

#ifdef IA64_USE_NEW_STUB
# define ASM_CLOBBERS_6	ASM_CLOBBERS_6_COMMON
#else
# define ASM_CLOBBERS_6	ASM_CLOBBERS_6_COMMON , "b7"
#endif

#if IS_IN(rtld)
# define DL_SYSINFO (void *) _dl_sysinfo_break
#else
#endif

#ifdef IA64_USE_NEW_STUB
static inline void *
__syscall_func (void)
{
# ifdef SHARED
  return ((tcbhead_t *)__thread_self)->__private;
# else
  extern uintptr_t _dl_sysinfo attribute_hidden;
  return (void *) _dl_sysinfo;
# endif
}
#endif

static inline long int
__internal_syscall0 (long int name)
{
  register long int r8 asm ("r8");
  register long int r10 asm ("r10");
  register long int r15 asm ("r15") = name;
#ifdef IA64_USE_NEW_STUB
  register void *b7 asm ("b7") = __syscall_func ();
  asm volatile ("br.call.sptk.many b6=%0;;\n"
		: "=b" (b7), "=r" (r8), "=r" (r10), "=r" (r15)
		: "0" (b7), "3" (r15)
		: "memory", "ar.pfs" ASM_CLOBBERS_0);
#else
  asm volatile (BREAK_INSN (__IA64_BREAK_SYSCALL)
		: "=r" (r8), "=r" (r10), "=r" (r15)
		: "2" (r15)
		: "memory" ASM_CLOBBERS_0);
#endif
  return r10 == -1 ? -r8 : r8;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  register long int out0 asm ("out0") = arg1;
  register long int r8 asm ("r8");
  register long int r10 asm ("r10");
  register long int r15 asm ("r15") = name;
#ifdef IA64_USE_NEW_STUB
  register void *b7 asm ("b7") = __syscall_func ();
  asm volatile ("br.call.sptk.many b6=%0;;\n"
		: "=b" (b7), "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0)
		: "0" (b7), "3" (r15), "4" (out0)
		: "memory", "ar.pfs" ASM_CLOBBERS_1);
#else
  asm volatile (BREAK_INSN (__IA64_BREAK_SYSCALL)
		: "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0)
		: "2" (r15), "3" (out0)
		: "memory" ASM_CLOBBERS_1);
#endif
  return r10 == -1 ? -r8 : r8;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  register long int out0 asm ("out0") = arg1;
  register long int out1 asm ("out1") = arg2;
  register long int r8 asm ("r8");
  register long int r10 asm ("r10");
  register long int r15 asm ("r15") = name;
#ifdef IA64_USE_NEW_STUB
  register void *b7 asm ("b7") = __syscall_func ();
  asm volatile ("br.call.sptk.many b6=%0;;\n"
		: "=b" (b7), "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0), "=r" (out1)
		: "0" (b7), "3" (r15), "4" (out0), "5" (out1)
		: "memory", "ar.pfs" ASM_CLOBBERS_2);
#else
  asm volatile (BREAK_INSN (__IA64_BREAK_SYSCALL)
		: "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0), "=r" (out1)
		: "2" (r15), "3" (out0), "4" (out1)
		: "memory" ASM_CLOBBERS_2);
#endif
  return r10 == -1 ? -r8 : r8;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register long int out0 asm ("out0") = arg1;
  register long int out1 asm ("out1") = arg2;
  register long int out2 asm ("out2") = arg3;
  register long int r8 asm ("r8");
  register long int r10 asm ("r10");
  register long int r15 asm ("r15") = name;
#ifdef IA64_USE_NEW_STUB
  register void *b7 asm ("b7") = __syscall_func ();
  asm volatile ("br.call.sptk.many b6=%0;;\n"
		: "=b" (b7), "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0), "=r" (out1), "=r" (out2)
		: "0" (b7), "3" (r15), "4" (out0), "5" (out1), "6" (out2)
		: "memory", "ar.pfs" ASM_CLOBBERS_3);
#else
  asm volatile (BREAK_INSN (__IA64_BREAK_SYSCALL)
		: "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0), "=r" (out1), "=r" (out2)
		: "2" (r15), "3" (out0), "4" (out1), "5" (out2)
		: "memory" ASM_CLOBBERS_3);
#endif
  return r10 == -1 ? -r8 : r8;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  register long int out0 asm ("out0") = arg1;
  register long int out1 asm ("out1") = arg2;
  register long int out2 asm ("out2") = arg3;
  register long int out3 asm ("out3") = arg4;
  register long int r8 asm ("r8");
  register long int r10 asm ("r10");
  register long int r15 asm ("r15") = name;
#ifdef IA64_USE_NEW_STUB
  register void *b7 asm ("b7") = __syscall_func ();
  asm volatile ("br.call.sptk.many b6=%0;;\n"
		: "=b" (b7), "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0), "=r" (out1), "=r" (out2), "=r" (out3)
		: "0" (b7), "3" (r15), "4" (out0), "5" (out1), "6" (out2),
		  "7" (out3)
		: "memory", "ar.pfs" ASM_CLOBBERS_4);
#else
  asm volatile (BREAK_INSN (__IA64_BREAK_SYSCALL)
		: "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0), "=r" (out1), "=r" (out2), "=r" (out3)
		: "2" (r15), "3" (out0), "4" (out1), "5" (out2), "6" (out3)
		: "memory" ASM_CLOBBERS_4);
#endif
  return r10 == -1 ? -r8 : r8;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  register long int out0 asm ("out0") = arg1;
  register long int out1 asm ("out1") = arg2;
  register long int out2 asm ("out2") = arg3;
  register long int out3 asm ("out3") = arg4;
  register long int out4 asm ("out4") = arg5;
  register long int r8 asm ("r8");
  register long int r10 asm ("r10");
  register long int r15 asm ("r15") = name;
#ifdef IA64_USE_NEW_STUB
  register void *b7 asm ("b7") = __syscall_func ();
  asm volatile ("br.call.sptk.many b6=%0;;\n"
		: "=b" (b7), "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0), "=r" (out1), "=r" (out2), "=r" (out3),
		  "=r" (out4)
		: "0" (b7), "3" (r15), "4" (out0), "5" (out1), "6" (out2),
		  "7" (out3), "8" (out4)
		: "memory", "ar.pfs" ASM_CLOBBERS_5);
#else
  asm volatile (BREAK_INSN (__IA64_BREAK_SYSCALL)
		: "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0), "=r" (out1), "=r" (out2), "=r" (out3),
		  "=r" (out4)
		: "2" (r15), "3" (out0), "4" (out1), "5" (out2), "6" (out3),
		  "7" (out4)
		: "memory" ASM_CLOBBERS_5);
#endif
  return r10 == -1 ? -r8 : r8;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  register long int out0 asm ("out0") = arg1;
  register long int out1 asm ("out1") = arg2;
  register long int out2 asm ("out2") = arg3;
  register long int out3 asm ("out3") = arg4;
  register long int out4 asm ("out4") = arg5;
  register long int out5 asm ("out5") = arg6;
  register long int r8 asm ("r8");
  register long int r10 asm ("r10");
  register long int r15 asm ("r15") = name;
#ifdef IA64_USE_NEW_STUB
  register void *b7 asm ("b7") = __syscall_func ();
  asm volatile ("br.call.sptk.many b6=%0;;\n"
		: "=b" (b7), "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0), "=r" (out1), "=r" (out2), "=r" (out3),
		  "=r" (out4), "=r" (out5)
		: "0" (b7), "3" (r15), "4" (out0), "5" (out1), "6" (out2),
		  "7" (out3), "8" (out4), "9" (out5)
		: "memory", "ar.pfs" ASM_CLOBBERS_6);
#else
  asm volatile (BREAK_INSN (__IA64_BREAK_SYSCALL)
		: "=r" (r8), "=r" (r10), "=r" (r15),
		  "=r" (out0), "=r" (out1), "=r" (out2), "=r" (out3),
		  "=r" (out4), "=r" (out5)
		: "2" (r15), "3" (out0), "4" (out1), "5" (out2), "6" (out3),
		  "7" (out4), "8" (out5)
		: "memory" ASM_CLOBBERS_6);
#endif
  return r10 == -1 ? -r8 : r8;
}

#undef HAVE_INTERNAL_BRK_ADDR_SYMBOL
#define HAVE_INTERNAL_BRK_ADDR_SYMBOL 1

#endif /* not __ASSEMBLER__ */

/* Pointer mangling support.  */
#if IS_IN (rtld)
/* We cannot use the thread descriptor because in ld.so we use setjmp
   earlier than the descriptor is initialized.  */
#else
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(reg, tmpreg) \
        add	tmpreg=-16,r13		\
        ;;				\
        ld8	tmpreg=[tmpreg]		\
        ;;				\
        xor	reg=reg, tmpreg
#  define PTR_DEMANGLE(reg, tmpreg) PTR_MANGLE (reg, tmpreg)
# else
#  define PTR_MANGLE(var) \
  (var) = (void *) ((uintptr_t) (var) ^ THREAD_GET_POINTER_GUARD ())
#  define PTR_DEMANGLE(var)	PTR_MANGLE (var)
# endif
#endif

#endif /* linux/ia64/sysdep.h */
