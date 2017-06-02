/* Find pathnames matching a pattern.  Compatibility version.
   Copyright (C) 2017 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include <glob.h>
#include "shlib-compat.h"

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2) \
    && !defined(GLOB_NO_OLD_VERSION)

#include <olddirent.h>

int __old_glob64 (const char *__pattern, int __flags,
		  int (*__errfunc) (const char *, int),
		  glob64_t *__pglob);

#define glob_t glob64_t
#define globfree(pglob) globfree64 (pglob)
#undef stat
#define stat stat64

#define dirent __old_dirent64
#define GL_READDIR(pglob, stream) \
  ((struct __old_dirent64 *) (pglob)->gl_readdir (stream))
#define __readdir(dirp) __old_readdir64 (dirp)
#define glob(pattern, flags, errfunc, pglob) \
  __old_glob64 (pattern, flags, errfunc, pglob)
#define convert_dirent __old_convert_dirent

#define GLOB_ATTRIBUTE attribute_compat_text_section

#include <posix/glob.c>

compat_symbol (libc, __old_glob64, glob64, GLIBC_2_1);
#endif
