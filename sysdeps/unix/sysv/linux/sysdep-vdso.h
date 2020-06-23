/* vDSO common definition for Linux.
   Copyright (C) 2015-2020 Free Software Foundation, Inc.
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

#ifndef SYSDEP_VDSO_LINUX_H
# define SYSDEP_VDSO_LINUX_H

#include <sysdep.h>
#include <ldsodefs.h>

typedef long int (*vdsop0_t)(void);
typedef long int (*vdsop1_t)(__syscall_arg_t);
typedef long int (*vdsop2_t)(__syscall_arg_t, __syscall_arg_t);
typedef long int (*vdsop3_t)(__syscall_arg_t, __syscall_arg_t,
			     __syscall_arg_t);
typedef long int (*vdsop4_t)(__syscall_arg_t, __syscall_arg_t,
			     __syscall_arg_t, __syscall_arg_t);
typedef long int (*vdsop5_t)(__syscall_arg_t, __syscall_arg_t,
			     __syscall_arg_t, __syscall_arg_t,
			     __syscall_arg_t);
typedef long int (*vdsop6_t)(__syscall_arg_t, __syscall_arg_t,
			     __syscall_arg_t, __syscall_arg_t,
			     __syscall_arg_t, __syscall_arg_t);
typedef long int (*vdsop7_t)(__syscall_arg_t, __syscall_arg_t,
			     __syscall_arg_t, __syscall_arg_t,
			     __syscall_arg_t, __syscall_arg_t,
			     __syscall_arg_t);

#include <sysdep-vdso-arch.h>

#define __internal_vsyscall_0(name) 					     \
  __internal_vsyscall0 ((vdsop0_t) GLRO(dl_vdso_##name), __NR_##name)
#define __internal_vsyscall_1(name, a1) 				     \
  __internal_vsyscall1 ((vdsop1_t) GLRO(dl_vdso_##name), __NR_##name,	     \
			ARGIFY (a1))
#define __internal_vsyscall_2(name, a1, a2) 				     \
  __internal_vsyscall2 ((vdsop2_t) GLRO(dl_vdso_##name), __NR_##name,	     \
			ARGIFY (a1), ARGIFY (a2))
#define __internal_vsyscall_3(name, a1, a2, a3) 			     \
  __internal_vsyscall3 ((vdsop3_t) GLRO(dl_vdso_##name), __NR_##name,	     \
			ARGIFY (a1), ARGIFY (a2), ARGIFY (a3))
#define __internal_vsyscall_4(name, a1, a2, a3, a4) 			     \
  __internal_vsyscall4 ((vdsop4_t) GLRO(dl_vdso_##name), __NR_##name,	     \
			ARGIFY (a1), ARGIFY (a2), ARGIFY (a3), ARGIFY (a4))
#define __internal_vsyscall_5(name, a1, a2, a3, a4, a5)			     \
  __internal_vsyscall5 ((vdsop5_t) GLRO(dl_vdso_##name), __NR_##name,	     \
			ARGIFY (a1), ARGIFY (a2), ARGIFY (a3), ARGIFY (a4),  \
			ARGIFY (a5))
#define __internal_vsyscall_6(name, a1, a2, a3, a4, a5, a6)		     \
  __internal_vsyscall6 ((vdsop6_t) GLRO(dl_vdso_##name), __NR_##name,	     \
			ARGIFY (a1), ARGIFY (a2), ARGIFY (a3), ARGIFY (a4),  \
			ARGIFY (a5), ARGIFY (a6))

#define internal_vsyscall(...)						     \
  __INTERNAL_SYSCALL_DISP(__internal_vsyscall_,__VA_ARGS__)

#define inline_vsyscall(...)						     \
  __syscall_ret (internal_vsyscall (__VA_ARGS__))

#endif /* SYSDEP_VDSO_LINUX_H  */
