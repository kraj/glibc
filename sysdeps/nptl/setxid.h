/* Copyright (C) 2004-2020 Free Software Foundation, Inc.
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

#include <nptl/pthreadP.h>
#include <sysdep.h>

static inline int (*get_nptl_setxid(void))(struct xid_command *cmdp)
{
# ifdef SHARED
  if (__glibc_likely (__libc_pthread_functions_init))
    return PTHFCT_PTR (ptr__nptl_setxid);
  return NULL;
# else
  extern __typeof (__nptl_setxid) __nptl_setxid __attribute__((weak));
  return __nptl_setxid;
# endif
}

static inline long int
__inline_setxid_syscall1 (int name, __syscall_arg_t arg1)
{
  int (*nptl_setxid) (struct xid_command *cmdp) = get_nptl_setxid ();
  if (nptl_setxid != NULL)
    {
      struct xid_command cmd = { name, .id = { arg1 }, };
      return nptl_setxid (&cmd);
    }
  return inline_syscall (name, arg1);
}

static inline long int
__inline_setxid_syscall2 (int name, __syscall_arg_t arg1,
			  __syscall_arg_t arg2)
{
  int (*nptl_setxid) (struct xid_command *cmdp) = get_nptl_setxid ();
  if (nptl_setxid != NULL)
    {
      struct xid_command cmd = { name, .id = { arg1, arg2 }, };
      return nptl_setxid (&cmd);
    }
  return inline_syscall (name, arg1, arg2);
}

static inline long int
__inline_setxid_syscall3 (int name, __syscall_arg_t arg1,
			   __syscall_arg_t arg2, __syscall_arg_t arg3)
{
  int (*nptl_setxid) (struct xid_command *cmdp) = get_nptl_setxid ();
  if (nptl_setxid != NULL)
    {
      struct xid_command cmd = { name, .id = { arg1, arg2, arg3 }, };
      return nptl_setxid (&cmd);
    }
  return inline_syscall (name, arg1, arg2, arg3);
}

#define __inline_setxid_syscall_0(name) 				\
  __inline_setxid_syscall0 (name)
#define __inline_setxid_syscall_1(name, a1) 				\
  __inline_setxid_syscall1 (name, ARGIFY (a1))
#define __inline_setxid_syscall_2(name, a1, a2) 			\
  __inline_setxid_syscall2 (name, ARGIFY (a1), ARGIFY (a2))
#define __inline_setxid_syscall_3(name, a1, a2, a3) 			\
  __inline_setxid_syscall3 (name, ARGIFY (a1), ARGIFY (a2), ARGIFY (a3))

#define __INLINE_SETXID_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __INLINE_SETXID_NARGS(...) \
  __INLINE_SETXID_NARGS_X (__VA_ARGS__,7,6,5,4,3,2,1,0,)
#define __INLINE_SETXID_CONCAT_X(a,b)     a##b
#define __INLINE_SETXID_CONCAT(a,b)       __INLINE_SETXID_CONCAT_X (a, b)
#define __INLINE_SETXID_DISP(b,...) \
  __INLINE_SETXID_CONCAT (b,__INLINE_SETXID_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define inline_setxid_syscall(...)					\
  __INLINE_SETXID_DISP (__inline_setxid_syscall_, __VA_ARGS__)
