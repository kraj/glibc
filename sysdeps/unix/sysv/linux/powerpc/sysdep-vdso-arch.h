/* vDSO definition for Linux/PowerPC.
   Copyright (C) 2020 Free Software Foundation, Inc.
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

/* The PowerPC vDSO symbols have the same semantic as syscalls where
   they CR0.SO in case of a failure.  Since this is not easily modeled
   in C, it requires specific wrapper to emulate a function call and
   checks its return value.  */

static inline long int
__internal_vsyscall0 (vdsop0_t vdsop, long int name)
{
  if (vdsop != NULL)
    {
      register void *r0 asm ("r0") = vdsop;
      register long int r3 asm ("r3");
      asm volatile ("mtctr %0\n\t"
		    "bctrl\n\t"
		    "neg   9, %1\n\t"
		    "isel  %1, 9, %1, 3\n\t"
		    : "+r" (r0), "=r" (r3)
		    :
		    : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
		      "cr0", "ctr", "lr", "memory");
      if (r3 == 0 || r3 != -ENOSYS)
	return r3;
    }
  return internal_syscall (name);
}

static inline long int
__internal_vsyscall1 (vdsop1_t vdsop, long int name, __syscall_arg_t arg1)
{
  if (vdsop != NULL)
    {
      register void *r0 asm ("r0") = vdsop;
      register long int r3 asm ("r3") = arg1;
      asm volatile ("mtctr %0\n\t"
		    "bctrl\n\t"
		    "neg   9, %1\n\t"
		    "isel  %1, 9, %1, 3\n\t"
		    : "+r" (r0), "+r" (r3)
		    :
		    : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
		      "cr0", "ctr", "lr", "memory");
      if (r3 == 0 || r3 != -ENOSYS)
	return r3;
    }
  return internal_syscall (name, arg1);
}

static inline long int
__internal_vsyscall2 (vdsop2_t vdsop, long int name, __syscall_arg_t arg1,
		      __syscall_arg_t arg2)
{
  if (vdsop != NULL)
    {
      register void *r0 asm ("r0") = vdsop;
      register long int r3 asm ("r3") = arg1;
      register long int r4 asm ("r4") = arg2;
      asm volatile ("mtctr %0\n\t"
		    "bctrl\n\t"
		    "neg   9, %1\n\t"
		    "isel  %1, 9, %1, 3\n\t"
		    : "+r" (r0), "+r" (r3), "+r" (r4)
		    :
		    : "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12",
		      "cr0", "ctr", "lr", "memory");
      if (r3 == 0 || r3 != -ENOSYS)
	return r3;
    }
  return internal_syscall (name, arg1, arg2);
}

static inline long int
__internal_vsyscall3 (vdsop3_t vdsop, long int name,  __syscall_arg_t arg1,
		      __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  if (vdsop != NULL)
    {
      register void *r0 asm ("r0") = vdsop;
      register long int r3 asm ("r3") = arg1;
      register long int r4 asm ("r4") = arg2;
      register long int r5 asm ("r5") = arg3;
      asm volatile ("mtctr %0\n\t"
		    "bctrl\n\t"
		    "neg   9, %1\n\t"
		    "isel  %1, 9, %1, 3\n\t"
		    : "+r" (r0), "+r" (r3), "+r" (r4), "+r" (r5)
		    :
		    : "r6", "r7", "r8", "r9", "r10", "r11", "r12",
		      "cr0", "ctr", "lr", "memory");
      if (r3 == 0 || r3 != -ENOSYS)
	return r3;
    }
  return internal_syscall (name, arg1, arg2, arg3);
}

static inline long int
__internal_vsyscall4 (vdsop4_t vdsop, long int name,  __syscall_arg_t arg1,
		      __syscall_arg_t arg2, __syscall_arg_t arg3,
		      __syscall_arg_t arg4)
{
  if (vdsop != NULL)
    {
      register void *r0 asm ("r0") = vdsop;
      register long int r3 asm ("r3") = arg1;
      register long int r4 asm ("r4") = arg2;
      register long int r5 asm ("r5") = arg3;
      register long int r6 asm ("r6") = arg4;
      asm volatile ("mtctr %0\n\t"
		    "bctrl\n\t"
		    "neg   9, %1\n\t"
		    "isel  %1, 9, %1, 3\n\t"
		    : "+r" (r0), "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6)
		    :
		    : "r7", "r8", "r9", "r10", "r11", "r12",
		      "cr0", "ctr", "lr", "memory");
      if (r3 == 0 || r3 != -ENOSYS)
	return r3;
    }
  return internal_syscall (name, arg1, arg2, arg3, arg4);
}

static inline long int
__internal_vsyscall5 (vdsop5_t vdsop, long int name,  __syscall_arg_t arg1,
		      __syscall_arg_t arg2, __syscall_arg_t arg3,
		      __syscall_arg_t arg4, __syscall_arg_t arg5)
{
  if (vdsop != NULL)
    {
      register void *r0 asm ("r0") = vdsop;
      register long int r3 asm ("r3") = arg1;
      register long int r4 asm ("r4") = arg2;
      register long int r5 asm ("r5") = arg3;
      register long int r6 asm ("r6") = arg4;
      register long int r7 asm ("r7") = arg5;
      asm volatile ("mtctr %0\n\t"
		    "bctrl\n\t"
		    "neg   9, %1\n\t"
		    "isel  %1, 9, %1, 3\n\t"
		    : "+r" (r0), "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6),
		      "+r" (r7)
		    :
		    : "r8", "r9", "r10", "r11", "r12",
		      "cr0", "ctr", "lr", "memory");
      if (r3 == 0 || r3 != -ENOSYS)
	return r3;
    }
  return internal_syscall (name, arg1, arg2, arg3, arg4, arg5);
}

static inline long int
__internal_vsyscall6 (vdsop6_t vdsop, long int name,  __syscall_arg_t arg1,
		      __syscall_arg_t arg2, __syscall_arg_t arg3,
		      __syscall_arg_t arg4, __syscall_arg_t arg5,
		      __syscall_arg_t arg6)
{
  if (vdsop != NULL)
    {
      register void *r0 asm ("r0") = vdsop;
      register long int r3 asm ("r3") = arg1;
      register long int r4 asm ("r4") = arg2;
      register long int r5 asm ("r5") = arg3;
      register long int r6 asm ("r6") = arg4;
      register long int r7 asm ("r7") = arg5;
      register long int r8 asm ("r8") = arg6;
      asm volatile ("mtctr %0\n\t"
		    "bctrl\n\t"
		    "neg   9, %1\n\t"
		    "isel  %1, 9, %1, 3\n\t"
		    : "+r" (r0), "+r" (r3), "+r" (r4), "+r" (r5), "+r" (r6),
		      "+r" (r7), "+r" (r8)
		    :
		    : "r9", "r10", "r11", "r12",
		      "cr0", "ctr", "lr", "memory");
      if (r3 == 0 || r3 != -ENOSYS)
	return r3;
    }
  return internal_syscall (name, arg1, arg2, arg3, arg4, arg5, arg6);
}
