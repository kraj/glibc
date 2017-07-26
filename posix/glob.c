/* Copyright (C) 1991-2017 Free Software Foundation, Inc.
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

#ifndef _LIBC
/* Don't use __attribute__ __nonnull__ in this compilation unit.  Otherwise gcc
   optimizes away the pattern == NULL || pglob == NULL tests below.  */
# define _GL_ARG_NONNULL(params)
# include <config.h>
#endif

#include <glob.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Outcomment the following line for production quality code.  */
/* #define NDEBUG 1 */
#include <assert.h>

#include <stdio.h>		/* Needed on stupid SunOS for assert.  */

#include <unistd.h>
#if !defined POSIX && defined _POSIX_VERSION
# define POSIX
#endif

#if (defined _WIN32 || defined __WIN32__) && ! defined __CYGWIN__
# define WINDOWS32
#endif

#ifndef WINDOWS32
# include <pwd.h>
#endif

#include <errno.h>
#ifndef __set_errno
# define __set_errno(val) errno = (val)
#endif

#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#ifdef _LIBC
# undef strdup
# define strdup(str) __strdup (str)
# define sysconf(id) __sysconf (id)
# define closedir(dir) __closedir (dir)
# define opendir(name) __opendir (name)
# define readdir(str) __readdir64 (str)
# define getpwnam_r(name, bufp, buf, len, res) \
   __getpwnam_r (name, bufp, buf, len, res)
# ifndef __lstat64
#  define __lstat64(fname, buf) __lxstat64 (_STAT_VER, fname, buf)
# endif
# define struct_stat64		struct stat64
#else /* !_LIBC */
# define __getlogin_r(buf, len) getlogin_r (buf, len)
# define __stat64(fname, buf)   stat (fname, buf)
# define __fxstatat64(_, d, f, st, flag) fstatat (d, f, st, flag)
# define struct_stat64          struct stat
# ifndef __MVS__
#  define __alloca              alloca
# endif
# define __readdir              readdir
# define __glob_pattern_p       glob_pattern_p
# define COMPILE_GLOB64
#endif /* _LIBC */

#include <fnmatch.h>

#include <scratch_buffer.h>
#include "glob_internal.h"
#include <malloc/char_array-skeleton.c>

#ifndef LOGIN_NAME_MAX
# define LOGIN_NAME_MAX 256
#endif

static const char *next_brace_sub (const char *begin, int flags) __THROWNL;

/* A representation of a directory entry which does not depend on the
   layout of struct dirent, or the size of ino_t.  */
struct readdir_result
{
  const char *name;
# if defined _DIRENT_HAVE_D_TYPE || defined HAVE_STRUCT_DIRENT_D_TYPE
  uint8_t type;
# endif
  bool skip_entry;
};

# if defined _DIRENT_HAVE_D_TYPE || defined HAVE_STRUCT_DIRENT_D_TYPE
/* Initializer based on the d_type member of struct dirent.  */
#  define D_TYPE_TO_RESULT(source) (source)->d_type,

/* True if the directory entry D might be a symbolic link.  */
static bool
readdir_result_might_be_symlink (struct readdir_result d)
{
  return d.type == DT_UNKNOWN || d.type == DT_LNK;
}

/* True if the directory entry D might be a directory.  */
static bool
readdir_result_might_be_dir (struct readdir_result d)
{
  return d.type == DT_DIR || readdir_result_might_be_symlink (d);
}
# else /* defined _DIRENT_HAVE_D_TYPE || defined HAVE_STRUCT_DIRENT_D_TYPE */
#  define D_TYPE_TO_RESULT(source)

/* If we do not have type information, symbolic links and directories
   are always a possibility.  */

static bool
readdir_result_might_be_symlink (struct readdir_result d)
{
  return true;
}

static bool
readdir_result_might_be_dir (struct readdir_result d)
{
  return true;
}

# endif /* defined _DIRENT_HAVE_D_TYPE || defined HAVE_STRUCT_DIRENT_D_TYPE */

# if (defined POSIX || defined WINDOWS32) && !defined __GNU_LIBRARY__
/* Initializer for skip_entry.  POSIX does not require that the d_ino
   field be present, and some systems do not provide it. */
#  define D_INO_TO_RESULT(source) false,
# else
#  define D_INO_TO_RESULT(source) (source)->d_ino == 0,
# endif

/* Construct an initializer for a struct readdir_result object from a
   struct dirent *.  No copy of the name is made.  */
#define READDIR_RESULT_INITIALIZER(source) \
  {					   \
    source->d_name,			   \
    D_TYPE_TO_RESULT (source)		   \
    D_INO_TO_RESULT (source)		   \
  }

/* Call gl_readdir on STREAM.  This macro can be overridden to reduce
   type safety if an old interface version needs to be supported.  */
#ifndef GL_READDIR
# define GL_READDIR(pglob, stream) ((pglob)->gl_readdir (stream))
#endif

/* Extract name and type from directory entry.  No copy of the name is
   made.  If SOURCE is NULL, result name is NULL.  Keep in sync with
   convert_dirent64 below.  */
static struct readdir_result
convert_dirent (const struct dirent *source)
{
  if (source == NULL)
    {
      struct readdir_result result = { NULL, };
      return result;
    }
  struct readdir_result result = READDIR_RESULT_INITIALIZER (source);
  return result;
}

#ifndef COMPILE_GLOB64
/* Like convert_dirent, but works on struct dirent64 instead.  Keep in
   sync with convert_dirent above.  */
static struct readdir_result
convert_dirent64 (const struct dirent64 *source)
{
  if (source == NULL)
    {
      struct readdir_result result = { NULL, };
      return result;
    }
  struct readdir_result result = READDIR_RESULT_INITIALIZER (source);
  return result;
}
#endif


#ifndef attribute_hidden
# define attribute_hidden
#endif

#ifndef __attribute_noinline__
# if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 1)
#  define __attribute_noinline__ /* Ignore */
#else
#  define __attribute_noinline__ __attribute__ ((__noinline__))
# endif
#endif

#ifndef __glibc_unlikely
# define __glibc_unlikely(expr) __builtin_expect (expr, 0)
#endif

#ifndef _LIBC
/* The results of opendir() in this file are not used with dirfd and fchdir,
   and we do not leak fds to any single-threaded code that could use stdio,
   therefore save some unnecessary recursion in fchdir.c and opendir_safer.c.
   FIXME - if the kernel ever adds support for multi-thread safety for
   avoiding standard fds, then we should use opendir_safer.  */
# ifdef GNULIB_defined_opendir
#  undef opendir
# endif
# ifdef GNULIB_defined_closedir
#  undef closedir
# endif
#endif

static int glob_in_dir (const char *pattern, const char *directory,
			int flags, int (*errfunc) (const char *, int),
			glob_t *pglob);
extern int __glob_pattern_type (const char *pattern, int quote)
    attribute_hidden;

static int prefix_array (const char *prefix, char **array, size_t n) __THROWNL;
static int collated_compare (const void *, const void *) __THROWNL;


/* Find the end of the sub-pattern in a brace expression.  */
static const char *
next_brace_sub (const char *cp, int flags)
{
  size_t depth = 0;
  while (*cp != '\0')
    if ((flags & GLOB_NOESCAPE) == 0 && *cp == '\\')
      {
	if (*++cp == '\0')
	  break;
	++cp;
      }
    else
      {
	if ((*cp == '}' && depth-- == 0) || (*cp == ',' && depth == 0))
	  break;

	if (*cp++ == '{')
	  depth++;
      }

  return *cp != '\0' ? cp : NULL;
}

/* Obtain the full home directory path from user 'user_name' and writes it
   on char_array 'home_dir'.  */
static bool
get_home_directory (const char *user_name, struct char_array *home_dir)
{
  struct passwd *p;
#if defined HAVE_GETPWNAM_R || defined _LIBC
  struct passwd pwbuf;
  int save = errno;
  struct scratch_buffer pwtmpbuf;
  scratch_buffer_init (&pwtmpbuf);

  while (getpwnam_r (user_name, &pwbuf, pwtmpbuf.data, pwtmpbuf.length, &p)
	 != 0)
    {
      if (errno != ERANGE)
	{
	  p = NULL;
	  break;
	}
      if (!scratch_buffer_grow (&pwtmpbuf))
	return false;
      __set_errno (save);
    }
#else
  p = getpwnam (pwtmpbuf.data);
#endif

  bool retval = false;
  if (p != NULL)
    {
      if (char_array_set_str (home_dir, p->pw_dir))
	retval = true;
    }
  scratch_buffer_free (&pwtmpbuf);
  return retval;
}

/* Allocate '(size + incr) * typesize' bytes while for overflow on the
   arithmetic operations.  */
static void *
glob_malloc_incr (size_t size, size_t incr, size_t typesize)
{
  size_t newsize;
  if (check_add_overflow_size_t (size, incr, &newsize))
    return NULL;
  return __libc_reallocarray (NULL, newsize, typesize);
}

/* Allocate '(size + incr1 + incr2) * typesize' bytes while for overflow on
   the arithmetic operations.  */
static void *
glob_malloc_incr2 (size_t size, size_t incr1, size_t incr2, size_t typesize)
{
  size_t newsize;
  if (check_add_overflow_size_t (size, incr1, &newsize)
      || check_add_overflow_size_t (newsize, incr2, &newsize))
    return NULL;
  return __libc_reallocarray (NULL, newsize, typesize);
}

/* Reallocate '(size + incr1) * typesize' bytes while for overflow on the
   arithmetic operations.  */
static void *
glob_realloc_incr (void *old, size_t size, size_t incr, size_t typesize)
{
  size_t newsize;
  if (check_add_overflow_size_t (size, incr, &newsize))
    return NULL;
  return __libc_reallocarray (old, newsize, typesize);
}

/* Do glob searching for PATTERN, placing results in PGLOB.
   The bits defined above may be set in FLAGS.
   If a directory cannot be opened or read and ERRFUNC is not nil,
   it is called with the pathname that caused the error, and the
   'errno' value from the failing call; if it returns non-zero
   'glob' returns GLOB_ABORTED; if it returns zero, the error is ignored.
   If memory cannot be allocated for PGLOB, GLOB_NOSPACE is returned.
   Otherwise, 'glob' returns zero.  */
int
#ifdef GLOB_ATTRIBUTE
GLOB_ATTRIBUTE
#endif
glob (const char *pattern, int flags, int (*errfunc) (const char *, int),
      glob_t *pglob)
{
  const char *filename;
  size_t dirlen;
  int status;
  size_t oldcount;
  int meta;
  bool dirname_modified;
  /* Indicate if the directory should be prepended on return values.  */
  bool dirname_prefix = true;
  glob_t dirs;
  int retval = 0;
  struct char_array dirname;

  if (pattern == NULL || pglob == NULL || (flags & ~__GLOB_FLAGS) != 0)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (!char_array_init_empty (&dirname))
    return GLOB_NOSPACE;

  /* POSIX requires all slashes to be matched.  This means that with
     a trailing slash we must match only directories.  */
  if (pattern[0] && pattern[strlen (pattern) - 1] == '/')
    flags |= GLOB_ONLYDIR;

  if (!(flags & GLOB_DOOFFS))
    /* Have to do this so 'globfree' knows where to start freeing.  It
       also makes all the code that uses gl_offs simpler. */
    pglob->gl_offs = 0;

  if (!(flags & GLOB_APPEND))
    {
      pglob->gl_pathc = 0;
      if (!(flags & GLOB_DOOFFS))
	pglob->gl_pathv = NULL;
      else
	{
	  size_t i;

	  pglob->gl_pathv = glob_malloc_incr (pglob->gl_offs, 1,
					      sizeof (char *));
	  if (pglob->gl_pathv == NULL)
	    goto err_nospace;

	  for (i = 0; i <= pglob->gl_offs; ++i)
	    pglob->gl_pathv[i] = NULL;
	}
    }

  if (flags & GLOB_BRACE)
    {
      const char *begin;

      if (flags & GLOB_NOESCAPE)
	begin = strchr (pattern, '{');
      else
	{
	  begin = pattern;
	  while (1)
	    {
	      if (*begin == '\0')
		{
		  begin = NULL;
		  break;
		}

	      if (*begin == '\\' && begin[1] != '\0')
		++begin;
	      else if (*begin == '{')
		break;

	      ++begin;
	    }
	}

      if (begin != NULL)
	{
	  /* Allocate working buffer large enough for our work.  Note that
	     we have at least an opening and closing brace.  */
	  size_t firstc;
	  const char *p;
	  const char *next;
	  const char *rest;
	  size_t rest_len;
	  struct char_array onealt;

	  /* We know the prefix for all sub-patterns.  */
	  ptrdiff_t onealtlen = begin - pattern;
	  if (!char_array_init_str_size (&onealt, pattern, onealtlen))
	    {
	      if (!(flags & GLOB_APPEND))
		{
		  pglob->gl_pathc = 0;
		  pglob->gl_pathv = NULL;
		}
	      goto err_nospace;
	    }

	  /* Find the first sub-pattern and at the same time find the
	     rest after the closing brace.  */
	  next = next_brace_sub (begin + 1, flags);
	  if (next == NULL)
	    {
	      /* It is an invalid expression.  */
	      char_array_free (&onealt);
	      goto illegal_brace;
	    }

	  /* Now find the end of the whole brace expression.  */
	  rest = next;
	  while (*rest != '}')
	    {
	      rest = next_brace_sub (rest + 1, flags);
	      if (rest == NULL)
		{
		  /* It is an illegal expression.  */
		  char_array_free (&onealt);
		  goto illegal_brace;
		}
	    }
	  /* Please note that we now can be sure the brace expression
	     is well-formed.  */
	  rest_len = strlen (++rest) + 1;

	  /* We have a brace expression.  BEGIN points to the opening {,
	     NEXT points past the terminator of the first element, and END
	     points past the final }.  We will accumulate result names from
	     recursive runs for each brace alternative in the buffer using
	     GLOB_APPEND.  */
	  firstc = pglob->gl_pathc;

	  p = begin + 1;
	  while (1)
	    {
	      int result;

	      /* Construct the new glob expression.  */
	      ptrdiff_t nextlen = next - p;
	      if (!char_array_replace_str_pos (&onealt, onealtlen, p, nextlen)
		  || !char_array_replace_str_pos (&onealt, onealtlen + nextlen,
						  rest, rest_len))
		{
		  char_array_free (&onealt);
		  retval = GLOB_NOSPACE;
		  goto out;
		}

	      result = glob (char_array_str (&onealt),
			     ((flags & ~(GLOB_NOCHECK | GLOB_NOMAGIC))
			      | GLOB_APPEND), errfunc, pglob);

	      /* If we got an error, return it.  */
	      if (result && result != GLOB_NOMATCH)
		{
		  char_array_free (&onealt);
		  if (!(flags & GLOB_APPEND))
		    {
		      globfree (pglob);
		      pglob->gl_pathc = 0;
		    }
		  retval = result;
		  goto out;
		}

	      if (*next == '}')
		/* We saw the last entry.  */
		break;

	      p = next + 1;
	      next = next_brace_sub (p, flags);
	      assert (next != NULL);
	    }

	  char_array_free (&onealt);

	  if (pglob->gl_pathc != firstc)
	    /* We found some entries.  */
	    retval = 0;
	  else if (!(flags & (GLOB_NOCHECK|GLOB_NOMAGIC)))
	    retval = GLOB_NOMATCH;
	  goto out;
	}
    }

  oldcount = pglob->gl_pathc + pglob->gl_offs;

  /* Find the filename.  */
  filename = strrchr (pattern, '/');
#if defined __MSDOS__ || defined WINDOWS32
  /* The case of "d:pattern".  Since `:' is not allowed in
     file names, we can safely assume that wherever it
     happens in pattern, it signals the filename part.  This
     is so we could some day support patterns like "[a-z]:foo".  */
  if (filename == NULL)
    filename = strchr (pattern, ':');
#endif /* __MSDOS__ || WINDOWS32 */
  dirname_modified = false;
  if (filename == NULL)
    {
      /* This can mean two things: a simple name or "~name".  The latter
	 case is nothing but a notation for a directory.  */
      if ((flags & (GLOB_TILDE|GLOB_TILDE_CHECK)) && pattern[0] == '~')
	{
	  if (!char_array_set_str (&dirname, pattern))
	    goto err_nospace;
	  dirlen = strlen (pattern);

	  /* Set FILENAME to NULL as a special flag.  This is ugly but
	     other solutions would require much more code.  We test for
	     this special case below.  */
	  filename = NULL;
	}
      else
	{
	  if (__glibc_unlikely (pattern[0] == '\0'))
	    {
	      dirs.gl_pathv = NULL;
	      goto no_matches;
	    }

	  filename = pattern;
#ifdef _AMIGA
# define CURRENT_FILENAME ""
#else
# define CURRENT_FILENAME "."
#endif
	  if (!char_array_set_str (&dirname, CURRENT_FILENAME))
	    goto err_nospace;
	  dirlen = 0;
	}
    }
  else if (filename == pattern
	   || (filename == pattern + 1 && pattern[0] == '\\'
	       && (flags & GLOB_NOESCAPE) == 0))
    {
      /* "/pattern" or "\\/pattern".  */
      if (!char_array_set_str (&dirname, "/"))
	goto err_nospace;
      dirlen = 1;
      ++filename;
      /* prefix_array adds a separator for each result and DIRNAME is
	 already '/'.  So we indicate later that we should not prepend
	 anything for this specific case.  */
      dirname_prefix = false;
    }
  else
    {
      dirlen = filename - pattern;
#if defined __MSDOS__ || defined WINDOWS32
      if (*filename == ':'
	  || (filename > pattern + 1 && filename[-1] == ':'))
	{
	  char *drive_spec;

	  ++dirlen;
	  drive_spec = __alloca (dirlen + 1);
	  *((char *) mempcpy (drive_spec, pattern, dirlen)) = '\0';
	  /* For now, disallow wildcards in the drive spec, to
	     prevent infinite recursion in glob.  */
	  if (__glob_pattern_p (drive_spec, !(flags & GLOB_NOESCAPE)))
	    {
	      retval = GLOB_NOMATCH;
	      goto out;
	    }
	  /* If this is "d:pattern", we need to copy ':' to DIRNAME
	     as well.  If it's "d:/pattern", don't remove the slash
	     from "d:/", since "d:" and "d:/" are not the same.*/
	}
#endif
      if (!char_array_set_str_size (&dirname, pattern, dirlen))
	goto err_nospace;
      ++filename;

      if (filename[0] == '\0'
#if defined __MSDOS__ || defined WINDOWS32
	  && char_array_pos (&dirname, dirlen - 1) != ':'
	  && (dirlen < 3 || char_array_pos (&dirname, dirlen - 2) != ':'
	      || char_array_pos (&dirname, dirlen - 1) != '/')
#endif
	  && dirlen > 1)
	/* "pattern/".  Expand "pattern", appending slashes.  */
	{
	  int orig_flags = flags;
          int val;
	  if (!(flags & GLOB_NOESCAPE)
	      && char_array_pos (&dirname, dirlen - 1) == '\\')
	    {
	      /* "pattern\\/".  Remove the final backslash if it hasn't
		 been quoted.  */
	      size_t p = dirlen - 1;
	      while (p > 0 && char_array_pos (&dirname, p - 1) == '\\') --p;
	      if ((dirlen - p) & 1)
		{
		  /* Since we are shrinking the array, there is no need to
		     check the function return.  */
		  dirlen -= 1;
		  char_array_crop (&dirname, dirlen);
		  flags &= ~(GLOB_NOCHECK | GLOB_NOMAGIC);
		}
	    }
	  val = glob (char_array_str (&dirname), flags | GLOB_MARK, errfunc,
		      pglob);
	  if (val == 0)
	    pglob->gl_flags = ((pglob->gl_flags & ~GLOB_MARK)
			       | (flags & GLOB_MARK));
	  else if (val == GLOB_NOMATCH && flags != orig_flags)
	    {
	      /* Make sure globfree (&dirs); is a nop.  */
	      dirs.gl_pathv = NULL;
	      flags = orig_flags;
	      oldcount = pglob->gl_pathc + pglob->gl_offs;
	      goto no_matches;
	    }
	  retval = val;
	  goto out;
	}
    }

  if ((flags & (GLOB_TILDE|GLOB_TILDE_CHECK))
      && char_array_pos (&dirname, 0) == '~')
    {
      if (char_array_pos (&dirname, 1) == '\0'
	  || char_array_pos (&dirname, 1) == '/'
	  || (!(flags & GLOB_NOESCAPE) && char_array_pos (&dirname, 1) == '\\'
	      && (char_array_pos (&dirname, 2) == '\0'
		  || char_array_pos (&dirname, 2) == '/')))
	{
	  /* Look up home directory.  */
	  struct char_array home_dir;

	  const char *home_env = getenv ("HOME");
	  home_env = home_env == NULL ? "" : home_env;
	  if (!char_array_init_str (&home_dir, home_env))
	    {
	      retval = GLOB_NOSPACE;
	      goto out;
	    }
# ifdef _AMIGA
	  if (home_dir == NULL || home_dir[0] == '\0')
	    home_dir = "SYS:";
# else
#  ifdef WINDOWS32
	  /* Windows NT defines HOMEDRIVE and HOMEPATH.  But give preference
	     to HOME, because the user can change HOME.  */
	  if (home_dir == NULL || home_dir[0] == '\0')
	    {
	      const char *home_drive = getenv ("HOMEDRIVE");
	      const char *home_path = getenv ("HOMEPATH");

	      if (home_drive != NULL && home_path != NULL)
		{
		  size_t home_drive_len = strlen (home_drive);
		  size_t home_path_len = strlen (home_path);
		  char *mem = alloca (home_drive_len + home_path_len + 1);

		  memcpy (mem, home_drive, home_drive_len);
		  memcpy (mem + home_drive_len, home_path, home_path_len + 1);
		  home_dir = mem;
		}
	      else
		home_dir = "c:/users/default"; /* poor default */
	    }
#  else
	  if (char_array_is_empty (&home_dir))
	    {
	      int success;
	      char user_name[LOGIN_NAME_MAX];

	      success = __getlogin_r (user_name, sizeof (user_name)) == 0;
	      if (success)
		{
		  if (!get_home_directory (user_name, &home_dir))
		    goto err_nospace;
		}
	    }
	  if (char_array_is_empty (&home_dir))
	    {
	      if (flags & GLOB_TILDE_CHECK)
		{
		  retval = GLOB_NOMATCH;
		  goto out;
		}
	      else
		{
		  if (!char_array_set_str (&home_dir, "~"))
		    goto err_nospace;
		}
	    }
#  endif /* WINDOWS32 */
# endif
	  /* Now construct the full directory.  */
	  if (char_array_pos (&dirname, 1) == '\0')
	    {
	      if (!char_array_set_str (&dirname, char_array_str (&home_dir)))
		goto err_nospace;
	      dirlen = char_array_size (&dirname) - 1;
	    }
	  else
	    {
	      /* Replaces '~' by the obtained HOME dir.  */
	      char_array_erase (&dirname, 0);
	      if (!char_array_prepend_str (&dirname,
					   char_array_str (&home_dir)))
		goto err_nospace;
	    }
	  char_array_free (&home_dir);
	  dirname_modified = true;
	}
# if !defined _AMIGA && !defined WINDOWS32
      else
	{
	  char *dirnamestr = char_array_at (&dirname, 0);
	  char *end_name = strchr (dirnamestr, '/');
	  char user_name[LOGIN_NAME_MAX];
	  char *unescape = NULL;

	  if (!(flags & GLOB_NOESCAPE))
	    {
	      if (end_name == NULL)
		{
		  unescape = strchr (dirnamestr, '\\');
		  if (unescape)
		    end_name = strchr (unescape, '\0');
		}
	      else
		unescape = memchr (dirnamestr, '\\', end_name - dirnamestr);
	    }
	  if (end_name == NULL)
	    strncpy (user_name, dirnamestr + 1, LOGIN_NAME_MAX - 1);
	  else
	    {
	      if (unescape != NULL)
		{
		  ptrdiff_t name_len = unescape - dirnamestr - 1;
		  name_len = MIN (name_len, LOGIN_NAME_MAX - 1);
		  char *p = mempcpy (user_name, dirnamestr + 1, name_len);
		  char *q = unescape;
		  while (*q != '\0')
		    {
		      if (*q == '\\')
			{
			  if (q[1] == '\0')
			    {
			      /* "~fo\\o\\" unescape to user_name "foo\\",
				 but "~fo\\o\\/" unescape to user_name
				 "foo".  */
			      if (filename == NULL)
				*p++ = '\\';
			      break;
			    }
			  ++q;
			}
		      *p++ = *q++;
		    }
		  *p = '\0';
		}
	      else
		{
		  ptrdiff_t name_len = end_name - dirnamestr;
		  name_len = MIN (name_len, LOGIN_NAME_MAX - 1);
		  *((char *) mempcpy (user_name, dirnamestr + 1, name_len))
		    = '\0';
		}
	    }

	  /* Look up specific user's home directory.  */
	  if (get_home_directory (user_name, &dirname))
	    {
	      dirlen = char_array_size (&dirname) - 1;
	      dirname_modified = true;
	    }
	  else
	    {
	      if (flags & GLOB_TILDE_CHECK)
	       /* We have to regard it as an error if we cannot find the
		  home directory.  */
	        {
		  retval = GLOB_NOMATCH;
		  goto out;
		}
	    }
	}
# endif	/* Not Amiga && not WINDOWS32.  */
    }

  /* Now test whether we looked for "~" or "~NAME".  In this case we
     can give the answer now.  */
  if (filename == NULL)
    {
      struct stat st;
      struct_stat64 st64;

      /* Return the directory if we don't check for error or if it exists.  */
      if ((flags & GLOB_NOCHECK)
	  || (((__builtin_expect (flags & GLOB_ALTDIRFUNC, 0))
	       ? ((*pglob->gl_lstat) (char_array_str (&dirname), &st) == 0
		  && S_ISDIR (st.st_mode))
	       : (__lstat64 (char_array_str (&dirname), &st64) == 0
		  && S_ISDIR (st64.st_mode)))))
	{
	  char **new_gl_pathv;
	  size_t newcount;

	  if (check_add_overflow_size_t (pglob->gl_pathc, pglob->gl_offs,
					 &newcount))
	    {
	    nospace:
	      free (pglob->gl_pathv);
	      pglob->gl_pathv = NULL;
	      pglob->gl_pathc = 0;
	      goto err_nospace;
	    }

	  new_gl_pathv = glob_realloc_incr (pglob->gl_pathv, newcount, 2,
					    sizeof (char *));
	  if (new_gl_pathv == NULL)
	    goto nospace;
	  pglob->gl_pathv = new_gl_pathv;

	  if (flags & GLOB_MARK)
	    {
	      char *p;

	      pglob->gl_pathv[newcount] = glob_malloc_incr (dirlen, 2,
							    sizeof (char));
	      if (pglob->gl_pathv[newcount] == NULL)
		goto nospace;

	      p = mempcpy (pglob->gl_pathv[newcount],
			   char_array_str (&dirname), dirlen);
	      p[0] = '/';
	      p[1] = '\0';
	    }
	  else
	    {
	      pglob->gl_pathv[newcount] = strdup (char_array_str (&dirname));
	      if (pglob->gl_pathv[newcount] == NULL)
		goto nospace;
	    }
	  pglob->gl_pathv[++newcount] = NULL;
	  ++pglob->gl_pathc;
	  pglob->gl_flags = flags;

	  retval = 0;
	  goto out;
	}

      /* Not found.  */
      retval = GLOB_NOMATCH;
      goto out;
    }

  meta = __glob_pattern_type (char_array_str (&dirname),
			      !(flags & GLOB_NOESCAPE));
  /* meta is 1 if correct glob pattern containing metacharacters.
     If meta has bit (1 << 2) set, it means there was an unterminated
     [ which we handle the same, using fnmatch.  Broken unterminated
     pattern bracket expressions ought to be rare enough that it is
     not worth special casing them, fnmatch will do the right thing.  */
  if (meta & (__glob_special | __glob_bracket))
    {
      /* The directory name contains metacharacters, so we
	 have to glob for the directory, and then glob for
	 the pattern in each directory found.  */
      size_t i;

      if (!(flags & GLOB_NOESCAPE) && dirlen > 0
	  && char_array_pos (&dirname, dirlen - 1) == '\\')
	{
	  /* "foo\\/bar".  Remove the final backslash from dirname
	     if it has not been quoted.  */
	  size_t p = dirlen - 1;
	  while (p > 0 && char_array_pos (&dirname, p - 1) == '\\') --p;
	  if ((dirlen - p) & 1)
	    char_array_crop (&dirname, --dirlen);
	}

      if (__glibc_unlikely ((flags & GLOB_ALTDIRFUNC) != 0))
	{
	  /* Use the alternative access functions also in the recursive
	     call.  */
	  dirs.gl_opendir = pglob->gl_opendir;
	  dirs.gl_readdir = pglob->gl_readdir;
	  dirs.gl_closedir = pglob->gl_closedir;
	  dirs.gl_stat = pglob->gl_stat;
	  dirs.gl_lstat = pglob->gl_lstat;
	}

      status = glob (char_array_str (&dirname),
		     ((flags & (GLOB_ERR | GLOB_NOESCAPE
				| GLOB_ALTDIRFUNC))
		      | GLOB_NOSORT | GLOB_ONLYDIR),
		     errfunc, &dirs);
      if (status != 0)
	{
	  if ((flags & GLOB_NOCHECK) == 0 || status != GLOB_NOMATCH)
	    {
	      retval = status;
	      goto out;
	    }
	  goto no_matches;
	}

      /* We have successfully globbed the preceding directory name.
	 For each name we found, call glob_in_dir on it and FILENAME,
	 appending the results to PGLOB.  */
      for (i = 0; i < dirs.gl_pathc; ++i)
	{
	  size_t old_pathc;

#ifdef SHELL
	  {
	    /* Make globbing interruptible in the bash shell. */
	    extern int interrupt_state;

	    if (interrupt_state)
	      {
		globfree (&dirs);
		return GLOB_ABORTED;
	      }
	  }
#endif /* SHELL.  */

	  old_pathc = pglob->gl_pathc;
	  status = glob_in_dir (filename, dirs.gl_pathv[i],
				((flags | GLOB_APPEND)
				 & ~(GLOB_NOCHECK | GLOB_NOMAGIC)),
				errfunc, pglob);
	  if (status == GLOB_NOMATCH)
	    /* No matches in this directory.  Try the next.  */
	    continue;

	  if (status != 0)
	    {
	      globfree (&dirs);
	      globfree (pglob);
	      pglob->gl_pathc = 0;
	      retval = status;
	      goto out;
	    }

	  /* Stick the directory on the front of each name.  */
	  if (prefix_array (dirs.gl_pathv[i],
			    &pglob->gl_pathv[old_pathc + pglob->gl_offs],
			    pglob->gl_pathc - old_pathc))
	    {
	      globfree (&dirs);
	      globfree (pglob);
	      pglob->gl_pathc = 0;
	      goto err_nospace;
	    }
	}

      flags |= GLOB_MAGCHAR;

      /* We have ignored the GLOB_NOCHECK flag in the `glob_in_dir' calls.
	 But if we have not found any matching entry and the GLOB_NOCHECK
	 flag was set we must return the input pattern itself.  */
      if (pglob->gl_pathc + pglob->gl_offs == oldcount)
	{
	no_matches:
	  /* No matches.  */
	  if (flags & GLOB_NOCHECK)
	    {
	      size_t newcount;
	      char **new_gl_pathv;

	      if (check_add_overflow_size_t (pglob->gl_pathc, pglob->gl_offs,
					     &newcount))
		{
		nospace2:
		  globfree (&dirs);
		  goto err_nospace;
		}

	      new_gl_pathv = glob_realloc_incr (pglob->gl_pathv, newcount, 2,
						sizeof (char *));
	      if (new_gl_pathv == NULL)
		goto nospace2;
	      pglob->gl_pathv = new_gl_pathv;

	      pglob->gl_pathv[newcount] = strdup (pattern);
	      if (pglob->gl_pathv[newcount] == NULL)
		{
		  globfree (&dirs);
		  globfree (pglob);
		  pglob->gl_pathc = 0;
		  goto err_nospace;
		}

	      ++pglob->gl_pathc;
	      ++newcount;

	      pglob->gl_pathv[newcount] = NULL;
	      pglob->gl_flags = flags;
	    }
	  else
	    {
	      globfree (&dirs);
	      retval = GLOB_NOMATCH;
	      goto out;
	    }
	}

      globfree (&dirs);
    }
  else
    {
      size_t old_pathc = pglob->gl_pathc;
      int orig_flags = flags;

      if (meta & __glob_backslash)
	{
	  char *p = strchr (char_array_str (&dirname), '\\'), *q;
	  /* We need to unescape the dirname string.  It is certainly
	     allocated by alloca, as otherwise filename would be NULL
	     or dirname wouldn't contain backslashes.  */
	  q = p;
	  do
	    {
	      if (*p == '\\')
		{
		  *q = *++p;
		  --dirlen;
		}
	      else
		*q = *p;
	      ++q;
	    }
	  while (*p++ != '\0');
	  dirname_modified = true;
	}
      if (dirname_modified)
	flags &= ~(GLOB_NOCHECK | GLOB_NOMAGIC);
      status = glob_in_dir (filename, char_array_str (&dirname), flags,
			    errfunc, pglob);
      if (status != 0)
	{
	  if (status == GLOB_NOMATCH && flags != orig_flags
	      && pglob->gl_pathc + pglob->gl_offs == oldcount)
	    {
	      /* Make sure globfree (&dirs); is a nop.  */
	      dirs.gl_pathv = NULL;
	      flags = orig_flags;
	      goto no_matches;
	    }
	  retval = status;
	  goto out;
	}

      if (dirlen > 0)
	{
	  /* Stick the directory on the front of each name.  */
	  if (prefix_array (dirname_prefix ? char_array_str (&dirname) : "",
			    &pglob->gl_pathv[old_pathc + pglob->gl_offs],
			    pglob->gl_pathc - old_pathc))
	    {
	      globfree (pglob);
	      pglob->gl_pathc = 0;
	      goto err_nospace;
	    }
	}
    }

  if (flags & GLOB_MARK)
    {
      /* Append slashes to directory names.  */
      size_t i;
      struct stat st;
      struct_stat64 st64;

      for (i = oldcount; i < pglob->gl_pathc + pglob->gl_offs; ++i)
	if ((__builtin_expect (flags & GLOB_ALTDIRFUNC, 0)
	     ? ((*pglob->gl_lstat) (pglob->gl_pathv[i], &st) == 0
		&& (S_ISDIR (st.st_mode) || S_ISLNK (st.st_mode)))
	     : (__lstat64 (pglob->gl_pathv[i], &st64) == 0
		&& (S_ISDIR (st64.st_mode) || S_ISLNK (st64.st_mode)))))
	  {
	    size_t len = strlen (pglob->gl_pathv[i]);
	    char *new = glob_realloc_incr (pglob->gl_pathv[i], len, 2,
					   sizeof (char));
	    if (new == NULL)
	      {
		globfree (pglob);
		pglob->gl_pathc = 0;
		goto err_nospace;
	      }
	    strcpy (&new[len], "/");
	    pglob->gl_pathv[i] = new;
	  }
    }

  if (!(flags & GLOB_NOSORT))
    {
      /* Sort the vector.  */
      qsort (&pglob->gl_pathv[oldcount],
	     pglob->gl_pathc + pglob->gl_offs - oldcount,
	     sizeof (char *), collated_compare);
    }

 out:
  char_array_free (&dirname);
  return retval;

 err_nospace:
  char_array_free (&dirname);
  return GLOB_NOSPACE;

 illegal_brace:
  char_array_free (&dirname);
  return glob (pattern, flags & ~GLOB_BRACE, errfunc, pglob);
}
#if defined _LIBC && !defined glob
libc_hidden_def (glob)
#endif


/* Do a collated comparison of A and B.  */
static int
collated_compare (const void *a, const void *b)
{
  char *const *ps1 = a; char *s1 = *ps1;
  char *const *ps2 = b; char *s2 = *ps2;

  if (s1 == s2)
    return 0;
  if (s1 == NULL)
    return 1;
  if (s2 == NULL)
    return -1;
  return strcoll (s1, s2);
}


/* Prepend DIRNAME to each of N members of ARRAY, replacing ARRAY's
   elements in place.  Return nonzero if out of memory, zero if successful.
   A slash is inserted between DIRNAME and each elt of ARRAY,
   unless DIRNAME is just "/".  Each old element of ARRAY is freed.  */
static int
prefix_array (const char *dirname, char **array, size_t n)
{
  size_t i;
  size_t dirlen = strlen (dirname);
#if defined __MSDOS__ || defined WINDOWS32
  int sep_char = '/';
# define DIRSEP_CHAR sep_char
#else
# define DIRSEP_CHAR '/'
#endif

#if defined __MSDOS__ || defined WINDOWS32
  if (dirlen > 1)
    {
      if (dirname[dirlen - 1] == '/' && dirname[dirlen - 2] == ':')
	/* DIRNAME is "d:/".  Don't prepend the slash from DIRNAME.  */
	--dirlen;
      else if (dirname[dirlen - 1] == ':')
	{
	  /* DIRNAME is "d:".  Use `:' instead of `/'.  */
	  --dirlen;
	  sep_char = ':';
	}
    }
#endif

  for (i = 0; i < n; ++i)
    {
      size_t eltlen = strlen (array[i]) + 1;
      char *new = glob_malloc_incr2 (dirlen, 1, eltlen, sizeof (char));
      if (new == NULL)
	{
	  while (i > 0)
	    free (array[--i]);
	  return 1;
	}

      char *endp = mempcpy (new, dirname, dirlen);
      *endp++ = DIRSEP_CHAR;
      mempcpy (endp, array[i], eltlen);

      free (array[i]);
      array[i] = new;
    }

  return 0;
}

struct globnames_result
{
  char **names;
  size_t length;
};

/* Create a dynamic array for C string representing the glob name found.  */
#define DYNARRAY_STRUCT            globnames_array
#define DYNARRAY_ELEMENT_FREE(ptr) free (*ptr)
#define DYNARRAY_ELEMENT           char *
#define DYNARRAY_PREFIX            globnames_array_
#define DYNARRAY_FINAL_TYPE        struct globnames_result
#define DYNARRAY_INITIAL_SIZE      64
#include <malloc/dynarray-skeleton.c>

/* Like `glob', but PATTERN is a final pathname component,
   and matches are searched for in DIRECTORY.
   The GLOB_NOSORT bit in FLAGS is ignored.  No sorting is ever done.
   The GLOB_APPEND flag is assumed to be set (always appends).  */
static int
glob_in_dir (const char *pattern, const char *directory, int flags,
	     int (*errfunc) (const char *, int), glob_t *pglob)
{
  void *stream = NULL;
  struct globnames_array globnames;
  size_t nfound = 0;
  int meta;
  int save;
  int result;

  globnames_array_init (&globnames);

  meta = __glob_pattern_type (pattern, !(flags & GLOB_NOESCAPE));
  if (meta == __glob_none && (flags & (GLOB_NOCHECK|GLOB_NOMAGIC)))
    {
      /* We need not do any tests.  The PATTERN contains no meta
	 characters and we must not return an error therefore the
	 result will always contain exactly one name.  */
      flags |= GLOB_NOCHECK;
    }
  else if (meta == __glob_none)
    {
      /* Since we use the normal file functions we can also use stat()
	 to verify the file is there.  */
      union
      {
	struct stat st;
	struct_stat64 st64;
      } ust;
      struct char_array fullname;

      if (!char_array_init_str (&fullname, directory)
	  || !char_array_append_str (&fullname, "/")
	  || !char_array_append_str (&fullname, pattern))
	{
	  char_array_free (&fullname);
	  return GLOB_NOSPACE;
	}

      if ((__builtin_expect (flags & GLOB_ALTDIRFUNC, 0)
	   ? (*pglob->gl_lstat) (char_array_str (&fullname), &ust.st)
	   : __lstat64 (char_array_str (&fullname), &ust.st64)) == 0)
	/* We found this file to be existing.  Now tell the rest
	   of the function to copy this name into the result.  */
	flags |= GLOB_NOCHECK;

      char_array_free (&fullname);
    }
  else
    {
      stream = (__builtin_expect (flags & GLOB_ALTDIRFUNC, 0)
		? (*pglob->gl_opendir) (directory)
		: opendir (directory));
      if (stream == NULL)
	{
	  if (errno != ENOTDIR
	      && ((errfunc != NULL && (*errfunc) (directory, errno))
		  || (flags & GLOB_ERR)))
	    return GLOB_ABORTED;
	}
      else
	{
	  int fnm_flags = ((!(flags & GLOB_PERIOD) ? FNM_PERIOD : 0)
			   | ((flags & GLOB_NOESCAPE) ? FNM_NOESCAPE : 0)
#if defined _AMIGA || defined VMS
			   | FNM_CASEFOLD
#endif
			   );
	  flags |= GLOB_MAGCHAR;

	  while (1)
	    {
	      struct readdir_result d;
	      {
		if (__builtin_expect (flags & GLOB_ALTDIRFUNC, 0))
		  d = convert_dirent (GL_READDIR (pglob, stream));
		else
		  {
#ifdef COMPILE_GLOB64
		    d = convert_dirent (__readdir (stream));
#else
		    d = convert_dirent64 (__readdir64 (stream));
#endif
		  }
	      }
	      if (d.name == NULL)
		break;
	      if (d.skip_entry)
		continue;

	      /* If we shall match only directories use the information
		 provided by the dirent call if possible.  */
	      if ((flags & GLOB_ONLYDIR) && !readdir_result_might_be_dir (d))
		continue;

	      if (fnmatch (pattern, d.name, fnm_flags) == 0)
		{
		  globnames_array_add (&globnames, strdup (d.name));
		  if (globnames_array_has_failed (&globnames))
		    goto memory_error;
		  nfound++;
		}
	    }
	}
    }

  if (nfound == 0 && (flags & GLOB_NOCHECK))
    {
      size_t len = strlen (pattern);
      nfound = 1;
      char *newp = malloc (len + 1);
      if (newp == NULL)
	goto memory_error;
      *((char *) mempcpy (newp, pattern, len)) = '\0';
      globnames_array_add (&globnames, newp);
      if (globnames_array_has_failed (&globnames))
	goto memory_error;
    }

  result = GLOB_NOMATCH;
  if (nfound != 0)
    {
      char **new_gl_pathv;
      size_t newlen;
      result = 0;

      if (check_add_overflow_size_t (pglob->gl_pathc, pglob->gl_offs, &newlen)
	  || check_add_overflow_size_t (newlen, nfound, &newlen)
	  || check_add_overflow_size_t (newlen, 1, &newlen))
	goto memory_error;

      new_gl_pathv = realloc (pglob->gl_pathv, newlen * sizeof (char *));

      if (new_gl_pathv == NULL)
	{
	memory_error:
	  globnames_array_free (&globnames);
	  result = GLOB_NOSPACE;
	}
      else
	{
	  struct globnames_result ret = { .names = 0, .length = -1 };
	  if (!globnames_array_finalize (&globnames, &ret))
	    result = GLOB_NOSPACE;
	  else
	    {
	      for (size_t i = 0; i < ret.length; ++i)
		new_gl_pathv[pglob->gl_offs + pglob->gl_pathc++]
		  = ret.names[i];

	      pglob->gl_pathv = new_gl_pathv;
	      pglob->gl_pathv[pglob->gl_offs + pglob->gl_pathc] = NULL;
	      pglob->gl_flags = flags;
	    }
	  free (ret.names);
	}
    }

  if (stream != NULL)
    {
      save = errno;
      if (__glibc_unlikely (flags & GLOB_ALTDIRFUNC))
	(*pglob->gl_closedir) (stream);
      else
	closedir (stream);
      __set_errno (save);
    }

  return result;
}
