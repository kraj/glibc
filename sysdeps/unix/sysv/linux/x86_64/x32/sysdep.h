/* Copyright (C) 2011 Free Software Foundation, Inc.
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

#ifndef _LINUX_X32_SYSDEP_H
#define _LINUX_X32_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/sysv/linux/x86_64/sysdep.h>
#include <sysdeps/x86_64/x32/sysdep.h>

#ifdef __ASSEMBLER__

# ifdef SYSCALL_RETURN_INT64
#  define SYSCALL_SET_ERROR_RETURN orq $-1, %rax
# else
#  define SYSCALL_SET_ERROR_RETURN orl $-1, %eax
# endif

# ifndef PIC
/* Nothing here.  */
# elif RTLD_PRIVATE_ERRNO
#  undef SYSCALL_ERROR_HANDLER
#  define SYSCALL_ERROR_HANDLER			\
0:						\
  lea rtld_errno(%rip), %ecx;			\
  xorl %edx, %edx;				\
  subl %eax, %edx;				\
  movl %edx, (%rcx);				\
  SYSCALL_SET_ERROR_RETURN;			\
  jmp L(pseudo_end);
# else
#  undef SYSCALL_ERROR_HANDLER
#  define SYSCALL_ERROR_HANDLER			\
0:						\
  movq SYSCALL_ERROR_ERRNO@GOTTPOFF(%rip), %rcx;\
  xorl %edx, %edx;				\
  subl %eax, %edx;				\
  movl %edx, %fs:(%rcx);			\
  SYSCALL_SET_ERROR_RETURN;			\
  jmp L(pseudo_end);
# endif	/* PIC */

#endif	/* __ASSEMBLER__ */

/* Pointer mangling support.  */
#undef PTR_MANGLE
#undef PTR_DEMANGLE

#if defined NOT_IN_libc && defined IS_IN_rtld
/* We cannot use the thread descriptor because in ld.so we use setjmp
   earlier than the descriptor is initialized.  */
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(reg)	xorl __pointer_chk_guard_local(%rip), reg;    \
				roll $17, reg
#  define PTR_DEMANGLE(reg)	rorl $17, reg;				      \
				xorl __pointer_chk_guard_local(%rip), reg
# else
#  define PTR_MANGLE(reg)	asm ("xorl __pointer_chk_guard_local(%%rip), %0\n" \
				     "roll $17, %0"			      \
				     : "=r" (reg) : "0" (reg))
#  define PTR_DEMANGLE(reg)	asm ("rorl $17, %0\n"			      \
				     "xorl __pointer_chk_guard_local(%%rip), %0" \
				     : "=r" (reg) : "0" (reg))
# endif
#else
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(reg)	xorl %fs:POINTER_GUARD, reg;		      \
				roll $17, reg
#  define PTR_DEMANGLE(reg)	rorl $17, reg;				      \
				xorl %fs:POINTER_GUARD, reg
# else
#  define PTR_MANGLE(var)	asm ("xorl %%fs:%c2, %0\n"		      \
				     "roll $17, %0"			      \
				     : "=r" (var)			      \
				     : "0" (var),			      \
				       "i" (offsetof (tcbhead_t,	      \
						      pointer_guard)))
#  define PTR_DEMANGLE(var)	asm ("rorl $17, %0\n"			      \
				     "xorl %%fs:%c2, %0"		      \
				     : "=r" (var)			      \
				     : "0" (var),			      \
				       "i" (offsetof (tcbhead_t,	      \
						      pointer_guard)))
# endif
#endif

#endif /* linux/x86_64/x32/sysdep.h */
