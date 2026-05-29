/* Redirection of setjmp/longjmp inside the dynamic linker.
   Copyright (C) 2026 Free Software Foundation, Inc.
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

/* Some targets mangle ld.so's own setjmp/longjmp jmp_bufs with the global
   __pointer_chk_guard_local, because the dynamic linker's first uses of
   setjmp happen before the TCB is set up (see pointer_guard.h and the
   analysis of dl_main), while libc.so and the application mangle with the
   copy in the thread descriptor.  On such targets __pointer_chk_guard_local
   is a second copy of the process-wide pointer guard.  */

#ifndef _RTLD_SETJMP_H
#define _RTLD_SETJMP_H

#if IS_IN (rtld)
# include <pointer_guard.h> /* RTLD_USE_LIBC_SETJMP */
#endif

#ifdef RTLD_USE_LIBC_SETJMP

# include <setjmp.h>

extern __typeof (__sigsetjmp) *__rtld_setjmp attribute_hidden;
typedef void __rtld_longjmp_t (struct __jmp_buf_tag __env[1], int __val);
extern __rtld_longjmp_t *__rtld_longjmp attribute_hidden;

__extern_inline int __attribute__ ((__returns_twice__))
__sigsetjmp (struct __jmp_buf_tag __env[1], int __savemask)
{
  return __rtld_setjmp (__env, __savemask);
}

__extern_inline void __attribute__ ((__noreturn__))
__longjmp (__jmp_buf __env, int __val)
{
  __rtld_longjmp ((struct __jmp_buf_tag *) (void *) __env, __val);
  __builtin_unreachable ();
}

/* Activate the ld.so-local setjmp/longjmp.  Called after the first
   self-relocation, before any use of the exception handling.  */
void __rtld_setjmp_init_stubs (void) attribute_hidden;

/* Switch to the libc.so setjmp/longjmp.  Called while the RELRO variables
   are still writable, after libc.so has been relocated.  MAIN_MAP is the
   link map of the executable.  */
struct link_map;
void __rtld_setjmp_init_real (struct link_map *main_map) attribute_hidden;

#endif /* RTLD_USE_LIBC_SETJMP */

#endif /* _RTLD_SETJMP_H */
