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

/* Defines RTLD_PRIVATE_ERRNO.  */
#include <dl-sysdep.h>

#ifndef __ASSEMBLER__
# include <errno.h>
#endif

/* For Linux we can use the system call table in the header file
    /usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)   __NR_##syscall_name

#ifdef __ASSEMBLER__

/* In microblaze ABI function call arguments are passed in registers
   r5...r10. The return value is stored in r3 (or r3:r4 regiters pair).
   Linux syscall uses the same convention with the addition that the
   syscall number is passed in r12. To enter the kernel "brki r14,8"
   instruction is used.
   None of the abovementioned registers are presumed across function call
   or syscall.
*/
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
# undef SYSCALL_ERROR_LABEL
# ifdef PIC
#  define SYSCALL_ERROR_LABEL 0f
# else
#  define SYSCALL_ERROR_LABEL __syscall_error
# endif

# undef PSEUDO
# define PSEUDO(name, syscall_name, args)           \
  .text;                                            \
  ENTRY (name)                                      \
    DO_CALL (syscall_name, args);                   \
    addik r12,r0,-4095;                             \
    cmpu  r12,r12,r3;                               \
    bgei  r12,SYSCALL_ERROR_LABEL;

# undef PSEUDO_END
# define PSEUDO_END(name)                           \
  SYSCALL_ERROR_HANDLER;                            \
  END (name)

# undef PSEUDO_NOERRNO
# define PSEUDO_NOERRNO(name, syscall_name, args)   \
  .text;                                            \
  ENTRY (name)                                      \
    DO_CALL (syscall_name, args);

# undef PSEUDO_END_NOERRNO
# define PSEUDO_END_NOERRNO(name)                   \
  END (name)

/* The function has to return the error code.  */
# undef  PSEUDO_ERRVAL
# define PSEUDO_ERRVAL(name, syscall_name, args)    \
  .text;                                            \
  ENTRY (name)                                      \
    DO_CALL (syscall_name, args);                   \

# undef  PSEUDO_END_ERRVAL
# define PSEUDO_END_ERRVAL(name)                    \
  END (name)

# define ret_NOERRNO                                \
  rtsd r15,8; addk r0,r0,r0;

# define ret_ERRVAL                                 \
  rtsd r15,8; rsubk r3,r3,r0;

# ifdef PIC
#  define SYSCALL_ERROR_LABEL_DCL 0
#  if RTLD_PRIVATE_ERRNO
#   define SYSCALL_ERROR_HANDLER                    \
SYSCALL_ERROR_LABEL_DCL:                            \
    mfs   r12,rpc;                                  \
    addik r12,r12,_GLOBAL_OFFSET_TABLE_+8;          \
    lwi   r12,r12,rtld_errno@GOT;                   \
    rsubk r3,r3,r0;                                 \
    swi   r3,r12,0;                                 \
    rtsd  r15,8;                                    \
    addik r3,r0,-1;
#  else /* !RTLD_PRIVATE_ERRNO.  */
/* Store (-r3) into errno through the GOT.  */
#   if defined _LIBC_REENTRANT
#    define SYSCALL_ERROR_HANDLER                   \
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
#   else /* !_LIBC_REENTRANT.  */
#    define SYSCALL_ERROR_HANDLER                   \
SYSCALL_ERROR_LABEL_DCL:                            \
    mfs   r12,rpc;                                  \
    addik r12,r12,_GLOBAL_OFFSET_TABLE_+8;          \
    lwi   r12,r12,errno@GOT;                        \
    rsubk r3,r3,r0;                                 \
    swi   r3,r12,0;                                 \
    rtsd  r15,8;                                    \
    addik r3,r0,-1;
#    endif /* _LIBC_REENTRANT.  */
# endif /* RTLD_PRIVATE_ERRNO.  */
# else
#  define SYSCALL_ERROR_HANDLER  /* Nothing here; code in sysdep.S is used.  */
# endif /* PIC.  */

# define DO_CALL(syscall_name, args)                \
    addik r12,r0,SYS_ify (syscall_name);            \
    brki  r14,8;                                    \
    addk  r0,r0,r0;

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

#endif /* not __ASSEMBLER__ */

#endif /* _LINUX_MICROBLAZE_SYSDEP_H */
