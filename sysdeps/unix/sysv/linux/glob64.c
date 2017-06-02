/* Find pathnames matching a pattern.
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

#include <sys/types.h>

#ifdef __OFF_T_MATCHES_OFF64_T
# define glob64 __no_glob64_decl
# include <posix/glob.c>
# undef glob64
weak_alias (glob, glob64)
#else
# include <glob.h>
# include <dirent.h>
# include <sys/stat.h>

# define dirent dirent64
# define __readdir(dirp) __readdir64 (dirp)

# define glob_t glob64_t
# define glob(pattern, flags, errfunc, pglob) \
  __glob64 (pattern, flags, errfunc, pglob)
# define globfree(pglob) globfree64 (pglob)

# undef stat
# define stat stat64

# define COMPILE_GLOB64	1

# include <posix/glob.c>

# include "shlib-compat.h"

# ifdef GLOB_NO_OLD_VERSION
strong_alias (__glob64, glob64)
libc_hidden_def (glob64)
# else
versioned_symbol (libc, __glob64, glob64, GLIBC_2_2);
libc_hidden_ver (__glob64, glob64)
# endif
#endif
