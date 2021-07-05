/* Malloc debug DSO.
   Copyright (C) 2021 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <atomic.h>
#include <libc-symbols.h>
#include <shlib-compat.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>

#define TUNABLE_NAMESPACE malloc
#include <elf/dl-tunables.h>

/* A compatible libc will set this totem to a non-zero value.  This is
   needed for __malloc_check at the moment because the new __malloc_check_*
   functions are not available in older libc.  */
unsigned __malloc_debug_totem = 0;

/* Support only the glibc allocators.  */
extern void *__libc_malloc (size_t);
extern void __libc_free (void *);
extern void *__libc_realloc (void *, size_t);
extern void *__libc_memalign (size_t, size_t);
extern void *__libc_valloc (size_t);
extern void *__libc_pvalloc (size_t);
extern void *__libc_calloc (size_t, size_t);

#define DEBUG_FN(fn) \
  static __typeof (__libc_ ## fn) __debug_ ## fn

DEBUG_FN(malloc);
DEBUG_FN(free);
DEBUG_FN(realloc);
DEBUG_FN(memalign);
DEBUG_FN(valloc);
DEBUG_FN(pvalloc);
DEBUG_FN(calloc);

static bool malloc_called;

enum malloc_debug_hooks
{
  MALLOC_NONE_HOOK = 0,
  MALLOC_MCHECK_HOOK = 1 << 0, /* mcheck()  */
  MALLOC_MTRACE_HOOK = 1 << 1, /* mtrace()  */
  MALLOC_CHECK_HOOK = 1 << 2,  /* MALLOC_CHECK_ or glibc.malloc.check.  */
};
static unsigned __malloc_debugging_hooks;

static __always_inline bool
__is_malloc_debug_enabled (enum malloc_debug_hooks flag)
{
  return __malloc_debugging_hooks & flag;
}

static __always_inline void
__malloc_debug_enable (enum malloc_debug_hooks flag)
{
  __malloc_debugging_hooks |= flag;
}

static __always_inline void
__malloc_debug_disable (enum malloc_debug_hooks flag)
{
  __malloc_debugging_hooks &= ~flag;
}

#include "mcheck.c"
#include "mtrace.c"
#include "malloc-check.h"

extern void (*__free_hook) (void *, const void *);
compat_symbol_reference (libc, __free_hook, __free_hook, GLIBC_2_0);
extern void * (*__malloc_hook) (size_t, const void *);
compat_symbol_reference (libc, __malloc_hook, __malloc_hook, GLIBC_2_0);
extern void * (*__realloc_hook) (void *, size_t, const void *);
compat_symbol_reference (libc, __realloc_hook, __realloc_hook, GLIBC_2_0);
extern void * (*__memalign_hook) (size_t, size_t, const void *);
compat_symbol_reference (libc, __memalign_hook, __memalign_hook, GLIBC_2_0);

static size_t pagesize;

static void
TUNABLE_CALLBACK (set_mallopt_check) (tunable_val_t *valp)
{
  int32_t value = (int32_t) valp->numval;
  if (value != 0 && __malloc_debug_totem)
    __malloc_debug_enable (MALLOC_CHECK_HOOK);
}

static __always_inline void
maybe_initialize (void)
{
  if (!malloc_called)
    {
#if HAVE_TUNABLES
      TUNABLE_GET (check, int32_t, TUNABLE_CALLBACK (set_mallopt_check));
#else
      const char *s = secure_getenv ("MALLOC_CHECK_");
      if (s && s[0] != '\0' && s[0] != '0' && __malloc_debug_totem)
	__malloc_debug_enable (MALLOC_CHECK_HOOK);
#endif
      /* The mcheck initializer runs before this through the initializer
	 hook so it is safe to set this here.  */
      malloc_called = true;
    }
}

/* The allocator functions.  */

static void *
__debug_malloc (size_t bytes)
{
  void *(*hook) (size_t, const void *) = atomic_forced_read (__malloc_hook);
  if (__builtin_expect (hook != NULL, 0))
    return (*hook)(bytes, RETURN_ADDRESS (0));

  maybe_initialize ();

  void *victim = NULL;
  size_t orig_bytes = bytes;
  if ((!__is_malloc_debug_enabled (MALLOC_MCHECK_HOOK)
       || !malloc_mcheck_before (&bytes, &victim))
      && (!__is_malloc_debug_enabled (MALLOC_CHECK_HOOK)
	  || !__malloc_check_malloc (bytes, &victim)))
    {
      victim = __libc_malloc (bytes);
    }
  if (__is_malloc_debug_enabled (MALLOC_MCHECK_HOOK) && victim != NULL)
    victim = malloc_mcheck_after (victim, orig_bytes);
  if (__is_malloc_debug_enabled (MALLOC_MTRACE_HOOK))
    malloc_mtrace_after (victim, orig_bytes, RETURN_ADDRESS (0));

  return victim;
}
strong_alias (__debug_malloc, malloc)

static void
__debug_free (void *mem)
{
  void (*hook) (void *, const void *) = atomic_forced_read (__free_hook);
  if (__builtin_expect (hook != NULL, 0))
    {
      (*hook)(mem, RETURN_ADDRESS (0));
      return;
    }

  if (__is_malloc_debug_enabled (MALLOC_MCHECK_HOOK))
    mem = free_mcheck (mem);
  if (!__is_malloc_debug_enabled (MALLOC_CHECK_HOOK)
      || !__malloc_check_free (mem))
    {
      __libc_free (mem);
    }
  if (__is_malloc_debug_enabled (MALLOC_MTRACE_HOOK))
    free_mtrace (mem, RETURN_ADDRESS (0));
}
strong_alias (__debug_free, free)

static void *
__debug_realloc (void *oldmem, size_t bytes)
{
  void *(*hook) (void *, size_t, const void *) =
    atomic_forced_read (__realloc_hook);
  if (__builtin_expect (hook != NULL, 0))
    return (*hook)(oldmem, bytes, RETURN_ADDRESS (0));

  maybe_initialize ();

  size_t orig_bytes = bytes, oldsize = 0;
  void *victim = NULL;

  if ((!__is_malloc_debug_enabled (MALLOC_MCHECK_HOOK)
       || !realloc_mcheck_before (&oldmem, &bytes, &oldsize, &victim))
      && (!__is_malloc_debug_enabled (MALLOC_CHECK_HOOK)
	  || !__malloc_check_realloc (oldmem, bytes, &victim)))
    {
      victim = __libc_realloc (oldmem, bytes);
    }
  if (__is_malloc_debug_enabled (MALLOC_MCHECK_HOOK) && victim != NULL)
    victim = realloc_mcheck_after (victim, oldmem, orig_bytes,
				   oldsize);
  if (__is_malloc_debug_enabled (MALLOC_MTRACE_HOOK))
    realloc_mtrace_after (victim, oldmem, orig_bytes, RETURN_ADDRESS (0));

  return victim;
}
strong_alias (__debug_realloc, realloc)

static void *
_mid_memalign (size_t alignment, size_t bytes, const void *address)
{
  void *(*hook) (size_t, size_t, const void *) =
    atomic_forced_read (__memalign_hook);
  if (__builtin_expect (hook != NULL, 0))
    return (*hook)(alignment, bytes, address);

  maybe_initialize ();

  void *victim = NULL;
  size_t orig_bytes = bytes;

  if ((!__is_malloc_debug_enabled (MALLOC_MCHECK_HOOK)
       || !memalign_mcheck_before (alignment, &bytes, &victim))
      && (!__is_malloc_debug_enabled (MALLOC_CHECK_HOOK)
	  || !__malloc_check_memalign (alignment, bytes, &victim)))
    {
      victim = __libc_memalign (alignment, bytes);
    }
  if (__is_malloc_debug_enabled (MALLOC_MCHECK_HOOK) && victim != NULL)
    victim = memalign_mcheck_after (victim, alignment, orig_bytes);
  if (__is_malloc_debug_enabled (MALLOC_MTRACE_HOOK))
    memalign_mtrace_after (victim, orig_bytes, address);

  return victim;
}

static void *
__debug_memalign (size_t alignment, size_t bytes)
{
  return _mid_memalign (alignment, bytes, RETURN_ADDRESS (0));
}
strong_alias (__debug_memalign, memalign)
strong_alias (__debug_memalign, aligned_alloc)

static void *
__debug_pvalloc (size_t bytes)
{
  size_t rounded_bytes;

  if (!pagesize)
    pagesize = sysconf (_SC_PAGESIZE);

  /* ALIGN_UP with overflow check.  */
  if (__glibc_unlikely (__builtin_add_overflow (bytes,
						pagesize - 1,
						&rounded_bytes)))
    {
      errno = ENOMEM;
      return NULL;
    }
  rounded_bytes = rounded_bytes & -(pagesize - 1);

  return _mid_memalign (pagesize, rounded_bytes, RETURN_ADDRESS (0));
}
strong_alias (__debug_pvalloc, pvalloc)

static void *
__debug_valloc (size_t bytes)
{
  if (!pagesize)
    pagesize = sysconf (_SC_PAGESIZE);

  return _mid_memalign (pagesize, bytes, RETURN_ADDRESS (0));
}
strong_alias (__debug_valloc, valloc)

static int
__debug_posix_memalign (void **memptr, size_t alignment, size_t bytes)
{
  /* Test whether the SIZE argument is valid.  It must be a power of
     two multiple of sizeof (void *).  */
  if (alignment % sizeof (void *) != 0
      || !powerof2 (alignment / sizeof (void *))
      || alignment == 0)
    return EINVAL;

  *memptr = _mid_memalign (alignment, bytes, RETURN_ADDRESS (0));

  if (*memptr == NULL)
    return ENOMEM;

  return 0;
}
strong_alias (__debug_posix_memalign, posix_memalign)

static void *
__debug_calloc (size_t nmemb, size_t size)
{
  size_t bytes;

  if (__glibc_unlikely (__builtin_mul_overflow (nmemb, size, &bytes)))
    {
      errno = ENOMEM;
      return NULL;
    }

  void *(*hook) (size_t, const void *) = atomic_forced_read (__malloc_hook);
  if (__builtin_expect (hook != NULL, 0))
    {
      void *mem = (*hook)(bytes, RETURN_ADDRESS (0));

      if (mem != NULL)
	memset (mem, 0, bytes);

      return mem;
    }

  maybe_initialize ();

  size_t orig_bytes = bytes;
  void *victim = NULL;

  if ((!__is_malloc_debug_enabled (MALLOC_MCHECK_HOOK)
       || !malloc_mcheck_before (&bytes, &victim))
      && (!__is_malloc_debug_enabled (MALLOC_CHECK_HOOK)
	  || !__malloc_check_malloc (bytes, &victim)))
    {
      victim = __libc_malloc (bytes);
    }
  if (victim != NULL)
    {
      if (__is_malloc_debug_enabled (MALLOC_MCHECK_HOOK))
	victim = malloc_mcheck_after (victim, orig_bytes);
      memset (victim, 0, orig_bytes);
    }
  if (__is_malloc_debug_enabled (MALLOC_MTRACE_HOOK))
    malloc_mtrace_after (victim, orig_bytes, RETURN_ADDRESS (0));

  return victim;
}
strong_alias (__debug_calloc, calloc)

size_t
malloc_usable_size (void *mem)
{
  if (__is_malloc_debug_enabled (MALLOC_CHECK_HOOK))
    return __malloc_check_malloc_usable_size (mem);

  return __malloc_usable_size (mem);
}
