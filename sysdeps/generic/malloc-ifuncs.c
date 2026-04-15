/* Code for ifunc resolvers for malloc: generic version.
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

#if IS_IN (libc)

#include <malloc/malloc-internal.h>
#include <malloc-ifuncs.h>

# if HAVE_IFUNC

/* These resolvers are used by default unless overridden by a target.
   The target-specific resolvers must respect this logic of the default
   resolvers, replicating this logic where appropriate.

   Any aliases for malloc API functions must be defined here as well
   and re-defined along with the target-specific resolvers.  */

IFUNC_PROTO (__libc_malloc);
IFUNC_RESOLVER (__libc_malloc, void)
{
  return __libc_malloc_core;
}
strong_alias (__libc_malloc, malloc)

IFUNC_PROTO (__libc_calloc);
IFUNC_RESOLVER (__libc_calloc, void)
{
  return __libc_calloc_core;
}
weak_alias (__libc_calloc, calloc)

IFUNC_PROTO (__libc_memalign);
IFUNC_RESOLVER (__libc_memalign, void)
{
  return __libc_memalign_core;
}
weak_alias (__libc_memalign, memalign)

IFUNC_PROTO (__libc_valloc);
IFUNC_RESOLVER (__libc_valloc, void)
{
  return __libc_valloc_core;
}
weak_alias (__libc_valloc, valloc)

IFUNC_PROTO (__libc_pvalloc);
IFUNC_RESOLVER (__libc_pvalloc, void)
{
  return __libc_pvalloc_core;
}
weak_alias (__libc_pvalloc, pvalloc)

IFUNC_PROTO (__libc_realloc);
IFUNC_RESOLVER (__libc_realloc, void)
{
  return __libc_realloc_core;
}
strong_alias (__libc_realloc, realloc)

IFUNC_PROTO (__libc_free);
IFUNC_RESOLVER (__libc_free, void)
{
  return __libc_free_core;
}
# if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_26)
compat_symbol (libc, __libc_free, cfree, GLIBC_2_0);
# endif
strong_alias (__libc_free, free)

IFUNC_PROTO (__malloc_usable_size);
IFUNC_RESOLVER (__malloc_usable_size, void)
{
  return __malloc_usable_size_core;
}
weak_alias (__malloc_usable_size, malloc_usable_size)

IFUNC_PROTO (__posix_memalign);
IFUNC_RESOLVER (__posix_memalign, void)
{
  return __posix_memalign_core;
}
weak_alias (__posix_memalign, posix_memalign)

IFUNC_PROTO (__aligned_alloc);
IFUNC_RESOLVER (__aligned_alloc, void)
{
  return __aligned_alloc_core;
}
weak_alias (__aligned_alloc, aligned_alloc)

IFUNC_PROTO (__free_sized);
IFUNC_RESOLVER (__free_sized, void)
{
  return __free_sized_core;
}
weak_alias (__free_sized, free_sized)

IFUNC_PROTO (__free_aligned_sized);
IFUNC_RESOLVER (__free_aligned_sized, void)
{
  return __free_aligned_sized_core;
}
weak_alias (__free_aligned_sized, free_aligned_sized)

# endif /* HAVE_IFUNC */

#endif /* IS_IN (libc) */
