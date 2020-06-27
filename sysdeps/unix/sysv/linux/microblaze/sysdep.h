/* Copyright (C) 2000-2020 Free Software Foundation, Inc.

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

#ifndef _LINUX_MICROBLAZE_SYSDEP_H
#define _LINUX_MICROBLAZE_SYSDEP_H 1

#include <sysdeps/unix/sysdep.h>
#include <sysdeps/unix/sysv/linux/sysdep.h>
#include <sysdeps/microblaze/sysdep.h>

#ifdef __ASSEMBLER__

/* In microblaze ABI function call arguments are passed in registers
   r5...r10. The return value is stored in r3 (or r3:r4 regiters pair).
   Linux syscall uses the same convention with the addition that the
   syscall number is passed in r12. To enter the kernel "brki r14,8"
   instruction is used.
   None of the abovementioned registers are presumed across function call
   or syscall.
*/

/* We don't want the label for the error handler to be visible in the symbol
   table when we define it here.  */
# undef SYSCALL_ERROR_LABEL
# ifdef PIC
#  define SYSCALL_ERROR_LABEL 0f
# else
#  define SYSCALL_ERROR_LABEL __syscall_error
# endif

# ifdef PIC
#  define SYSCALL_ERROR_LABEL_DCL 0
/* Store (-r3) into errno through the GOT.  */
#  define SYSCALL_ERROR_HANDLER                     \
SYSCALL_ERROR_LABEL_DCL:                            \
    addik r1,r1,-16;                                \
    swi   r15,r1,0;                                 \
    swi   r20,r1,8;                                 \
    rsubk r3,r3,r0;                                 \
    swi   r3,r1,12;                                 \
    mfs   r20,rpc;                                  \
    addik r20,r20,_GLOBAL_OFFSET_TABLE_+8;          \
    brlid r15,__errno_location@PLT;                 \
    nop;                                            \
    lwi   r4,r1,12;                                 \
    swi   r4,r3,0;                                  \
    lwi   r20,r1,8;                                 \
    lwi   r15,r1,0;                                 \
    addik r1,r1,16;                                 \
    rtsd  r15,8;                                    \
    addik r3,r0,-1;
# else
#  define SYSCALL_ERROR_HANDLER  /* Nothing here; code in sysdep.S is used.  */
# endif /* PIC.  */

#else /* not __ASSEMBLER__ */

static inline long int
__internal_syscall0 (long int name)
{
  register long int ret asm ("r3");
  register long int r12 asm ("r12") = name;
  asm volatile ("brki r14,8; nop;"
		: "=r" (ret)
		: "r" (r12)
		: "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r4", "memory");
  return ret;
}

static inline long int
__internal_syscall1 (long int name, __syscall_arg_t arg1)
{
  register long int ret asm ("r3");
  register long int r12 asm ("r12") = name;
  register long int r5 asm ("r5") = arg1;
  asm volatile ("brki r14,8; nop;"
		: "=r" (ret)
		: "r" (r5), "r" (r12)
		: "r6", "r7", "r8", "r9", "r10", "r11", "r4", "memory");
  return ret;
}

static inline long int
__internal_syscall2 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2)
{
  register long int ret asm ("r3");
  register long int r12 asm ("r12") = name;
  register long int r5 asm ("r5") = arg1;
  register long int r6 asm ("r6") = arg2;
  asm volatile ("brki r14,8; nop;"
		: "=r" (ret)
		: "r" (r5), "r" (r6), "r" (r12)
		: "r7", "r8", "r9", "r10", "r11", "r4", "memory");
  return ret;
}

static inline long int
__internal_syscall3 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  register long int ret asm ("r3");
  register long int r12 asm ("r12") = name;
  register long int r5 asm ("r5") = arg1;
  register long int r6 asm ("r6") = arg2;
  register long int r7 asm ("r7") = arg3;
  asm volatile ("brki r14,8; nop;"
		: "=r" (ret)
		: "r" (r5), "r" (r6), "r" (r7), "r" (r12)
		: "r8", "r9", "r10", "r11", "r4", "memory");
  return ret;
}

static inline long int
__internal_syscall4 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4)
{
  register long int ret asm ("r3");
  register long int r12 asm ("r12") = name;
  register long int r5 asm ("r5") = arg1;
  register long int r6 asm ("r6") = arg2;
  register long int r7 asm ("r7") = arg3;
  register long int r8 asm ("r8") = arg4;
  asm volatile ("brki r14,8; nop;"
		: "=r" (ret)
		: "r" (r5), "r" (r6), "r" (r7), "r" (r8), "r" (r12)
		: "r9", "r10", "r11", "r4", "memory");
  return ret;
}

static inline long int
__internal_syscall5 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  register long int ret asm ("r3");
  register long int r12 asm ("r12") = name;
  register long int r5 asm ("r5") = arg1;
  register long int r6 asm ("r6") = arg2;
  register long int r7 asm ("r7") = arg3;
  register long int r8 asm ("r8") = arg4;
  register long int r9 asm ("r9") = arg5;
  asm volatile ("brki r14,8; nop;"
		: "=r" (ret)
		: "r" (r5), "r" (r6), "r" (r7), "r" (r8), "r" (r9), "r" (r12)
		: "r10", "r11", "r4", "memory");
  return ret;
}

static inline long int
__internal_syscall6 (long int name, __syscall_arg_t arg1,
		     __syscall_arg_t arg2, __syscall_arg_t arg3,
		     __syscall_arg_t arg4, __syscall_arg_t arg5,
		     __syscall_arg_t arg6)
{
  register long int ret asm ("r3");
  register long int r12 asm ("r12") = name;
  register long int r5 asm ("r5") = arg1;
  register long int r6 asm ("r6") = arg2;
  register long int r7 asm ("r7") = arg3;
  register long int r8 asm ("r8") = arg4;
  register long int r9 asm ("r9") = arg5;
  register long int r10 asm ("r10") = arg6;
  asm volatile ("brki r14,8; nop;"
		: "=r" (ret)
		: "r" (r5), "r" (r6), "r" (r7), "r" (r8), "r" (r9),
		  "r" (r10), "r" (r12)
		: "r11", "r4", "memory");
  return ret;
}

/* Pointer mangling is not yet supported for Microblaze.  */
# define PTR_MANGLE(var) (void) (var)
# define PTR_DEMANGLE(var) (void) (var)

# define SINGLE_THREAD_BY_GLOBAL	1

#undef HAVE_INTERNAL_BRK_ADDR_SYMBOL
#define HAVE_INTERNAL_BRK_ADDR_SYMBOL 1

#endif /* not __ASSEMBLER__ */

#endif /* _LINUX_MICROBLAZE_SYSDEP_H */
