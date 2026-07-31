/* Copyright (C) 1991-2026 Free Software Foundation, Inc.
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

#ifndef _LIBC

/* Don't use __attribute__ __nonnull__ in this compilation unit.  Otherwise gcc
   optimizes away the pattern == NULL test below.  */
# define _GL_ARG_NONNULL(params)

# include <libc-config.h>

#endif

#include <glob.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>

#if defined _WIN32 && ! defined __CYGWIN__
# define WINDOWS32
#endif

#ifndef WINDOWS32
# include <pwd.h>
#endif

#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#ifdef _LIBC
# undef strdup
# define strdup(str) __strdup (str)
# define sysconf(id) __sysconf (id)
# define closedir(dir) __closedir (dir)
# define opendir(name) __opendir (name)
# undef dirfd
# define dirfd(str) __dirfd (str)
# define readdir(str) __readdir64 (str)
# define getpwnam_r(name, bufp, buf, len, res) \
    __getpwnam_r (name, bufp, buf, len, res)
# define reallocarray(p, n, s) __libc_reallocarray (p, n, s)
# define FLEXIBLE_ARRAY_MEMBER
# ifndef struct_stat
#  define struct_stat           struct stat
# endif
# ifndef struct_stat64
#  define struct_stat64         struct stat64
# endif
# ifndef GLOB_LSTAT
#  define GLOB_LSTAT            gl_lstat
# endif
# ifndef GLOB_FSTATAT64
#  define GLOB_FSTATAT64        __fstatat64
# endif
# include <shlib-compat.h>
#else /* !_LIBC */
# define __glob                 glob
# define __getlogin_r(buf, len) getlogin_r (buf, len)
# define __fxstatat64(_, d, f, st, flag) fstatat (d, f, st, flag)
# ifndef __MVS__
#  define __alloca              alloca
# endif
# define __readdir              readdir
# define COMPILE_GLOB64
# define struct_stat            struct stat
# define struct_stat64          struct stat
# define GLOB_LSTAT             gl_lstat
# define GLOB_FSTATAT64         fstatat
#endif /* _LIBC */

#include <fnmatch.h>

#include <flexmember.h>
#include <glob_internal.h>
#include <scratch_buffer.h>

static const char *next_brace_sub (const char *begin, int flags) __THROWNL;

/* The type of ((struct dirent *) 0)->d_type is 'unsigned char' on most
   platforms, but 'unsigned int' in the mingw from mingw.org.  */
typedef uint_fast32_t dirent_type;

#if !defined _LIBC && !defined HAVE_STRUCT_DIRENT_D_TYPE
/* Any distinct values will do here.
   Undef any existing macros out of the way.  */
# undef DT_UNKNOWN
# undef DT_DIR
# undef DT_LNK
# define DT_UNKNOWN 0
# define DT_DIR 1
# define DT_LNK 2
#endif

/* A representation of a directory entry which does not depend on the
   layout of struct dirent, or the size of ino_t.  */
struct readdir_result
{
  const char *name;
#if defined _DIRENT_HAVE_D_TYPE || defined HAVE_STRUCT_DIRENT_D_TYPE
  dirent_type type;
#endif
};

/* Initialize and return type member of struct readdir_result.  */
static dirent_type
readdir_result_type (struct readdir_result d)
{
#if defined _DIRENT_HAVE_D_TYPE || defined HAVE_STRUCT_DIRENT_D_TYPE
# define D_TYPE_TO_RESULT(source) (source)->d_type,
  return d.type;
#else
# define D_TYPE_TO_RESULT(source)
  return DT_UNKNOWN;
#endif
}

/* Construct an initializer for a struct readdir_result object from a
   struct dirent *.  No copy of the name is made.  */
#define READDIR_RESULT_INITIALIZER(source) \
  {                                        \
    source->d_name,                        \
    D_TYPE_TO_RESULT (source)              \
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

#ifndef _LIBC
# include <alloca.h>

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

static int
glob_lstat (glob_t *pglob, int flags, const char *fullname)
{
/* Use on glob-lstat-compat.c to provide a compat symbol which does not
   use lstat / gl_lstat.  */
  union
  {
    struct_stat st;
    struct_stat64 st64;
  } ust;
  return (__glibc_unlikely (flags & GLOB_ALTDIRFUNC)
          ? pglob->GLOB_LSTAT (fullname, &ust.st)
          : GLOB_FSTATAT64 (AT_FDCWD, fullname, &ust.st64,
                            AT_SYMLINK_NOFOLLOW));
}

/* Set *R = A + B.  Return true if the answer is mathematically
   incorrect due to overflow; in this case, *R is the low order
   bits of the correct answer.  */

static bool
size_add_wrapv (size_t a, size_t b, size_t *r)
{
#if 7 <= __GNUC__ && !defined __ICC
  return __builtin_add_overflow (a, b, r);
#else
  *r = a + b;
  return *r < a;
#endif
}

static int glob_in_dir (const char *pattern, const char *directory,
                        int flags, int (*errfunc) (const char *, int),
                        glob_t *pglob);
static int prefix_array (const char *prefix, char **array, size_t n) __THROWNL;
static int collated_compare (const void *, const void *) __THROWNL;


/* Return true if FILENAME is a directory or a symbolic link to a directory.
   Use FLAGS and PGLOB to resolve the filename.  */
static bool
is_dir (char const *filename, int flags, glob_t const *pglob)
{
  struct_stat st;
  struct_stat64 st64;
  return (__glibc_unlikely (flags & GLOB_ALTDIRFUNC)
          ? pglob->gl_stat (filename, &st) == 0 && S_ISDIR (st.st_mode)
          : (GLOB_FSTATAT64 (AT_FDCWD, filename, &st64, 0) == 0
             && S_ISDIR (st64.st_mode)));
}

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

#ifndef WINDOWS32
static char *
glob_dup_pw_dir (const struct passwd *p, bool *nospace)
{
  char *result = NULL;

  if (p != NULL)
    {
      result = strdup (p->pw_dir);
      if (result == NULL)
        *nospace = true;
    }
  return result;
}

/* Return the home directory of the current user as a malloc'ed string, or
   NULL if it cannot be determined.  Set *NOSPACE if the failure was caused
   by an allocation failure.  */
static char *
glob_current_home_dir (bool *nospace)
{
  struct passwd *p;
# if defined HAVE_GETPWNAM_R || defined _LIBC
  struct passwd pwbuf;
# endif
  int err;

  *nospace = false;

  struct scratch_buffer s;
  scratch_buffer_init (&s);
  while (true)
    {
      p = NULL;
      err = __getlogin_r (s.data, s.length);
      if (err == 0)
        {
# if defined HAVE_GETPWNAM_R || defined _LIBC
          size_t ssize = strlen (s.data) + 1;
          char *sdata = s.data;
          err = getpwnam_r (sdata, &pwbuf, sdata + ssize, s.length - ssize,
                            &p);
# else
          p = getpwnam (s.data);
          if (p == NULL)
            err = errno;
# endif
        }
      if (err != ERANGE)
        break;
      if (!scratch_buffer_grow (&s))
        {
          /* scratch_buffer_grow has already released the buffer.  */
          *nospace = true;
          return NULL;
        }
    }

  char *result = err == 0 ? glob_dup_pw_dir (p, nospace) : NULL;
  scratch_buffer_free (&s);
  return result;
}

/* Likewise, for the home directory of the user named USER_NAME.  */
static char *
glob_user_home_dir (const char *user_name, bool *nospace)
{
  struct passwd *p;
  struct scratch_buffer pwtmpbuf;
  char *result;

  *nospace = false;
  scratch_buffer_init (&pwtmpbuf);

# if defined HAVE_GETPWNAM_R || defined _LIBC
  struct passwd pwbuf;

  while (getpwnam_r (user_name, &pwbuf, pwtmpbuf.data, pwtmpbuf.length, &p)
         == ERANGE)
    if (!scratch_buffer_grow (&pwtmpbuf))
      {
        /* scratch_buffer_grow has already released the buffer.  */
        *nospace = true;
        return NULL;
      }
# else
  p = getpwnam (user_name);
# endif

  result = glob_dup_pw_dir (p, nospace);
  scratch_buffer_free (&pwtmpbuf);
  return result;
}
#endif /* !WINDOWS32 */

#ifndef GLOB_ATTRIBUTE
# define GLOB_ATTRIBUTE
#endif

static int glob_internal (const char *pattern, int flags,
			  int (*errfunc) (const char *, int), glob_t *pglob)
     GLOB_ATTRIBUTE;

/* Do glob searching for PATTERN, placing results in PGLOB.
   The bits defined above may be set in FLAGS.
   If a directory cannot be opened or read and ERRFUNC is not nil,
   it is called with the pathname that caused the error, and the
   'errno' value from the failing call; if it returns non-zero
   'glob' returns GLOB_ABORTED; if it returns zero, the error is ignored.
   If memory cannot be allocated for PGLOB, GLOB_NOSPACE is returned.
   Otherwise, 'glob' returns zero.  */
int
GLOB_ATTRIBUTE
__glob (const char *pattern, int flags, int (*errfunc) (const char *, int),
        glob_t *pglob)
{
  return glob_internal (pattern, flags, errfunc, pglob);
}

/* Remove one unquoted trailing backslash from P, which is LEN bytes
   long, and return the new length.  */
static size_t
glob_strip_backslash (char *p, size_t len)
{
  char *q = &p[len - 1];

  while (q > p && q[-1] == '\\')
    --q;
  /* Check if the number of '\' is odd.  */
  if ((&p[len] - q) & 1)
    p[--len] = '\0';
  return len;
}

/* Remove the quoting backslashes from P, which is LEN bytes long and
   contains at least one backslash.  Return the new length.  */
static size_t
glob_unescape (char *p, size_t len)
{
  char *s = memchr (p, '\\', len);
  char *d = s;

  do
    {
      if (*s == '\\')
        {
          *d = *++s;
          --len;
        }
      else
        *d = *s;
      ++d;
    }
  while (*s++ != '\0');
  return len;
}

/* Append a slash to every name in PGLOB from OLDCOUNT on that names a
   directory (used by GLOB_MARK).  */
static bool
glob_mark_dirs (glob_t *pglob, size_t oldcount, int flags)
{
  for (size_t i = oldcount; i < pglob->gl_pathc + pglob->gl_offs; ++i)
    if (is_dir (pglob->gl_pathv[i], flags, pglob))
      {
        size_t len = strlen (pglob->gl_pathv[i]);
        char *new = realloc (pglob->gl_pathv[i], len + 2);
        if (new == NULL)
          return false;
        new[len] = '/';
        new[len + 1] = '\0';
        pglob->gl_pathv[i] = new;
      }
  return true;
}

static glob_t
glob_copy_dirfuncs (const glob_t *from, int flags)
{
  glob_t to = { 0 };

  if (__glibc_unlikely ((flags & GLOB_ALTDIRFUNC) != 0))
    {
      to.gl_opendir = from->gl_opendir;
      to.gl_readdir = from->gl_readdir;
      to.gl_closedir = from->gl_closedir;
      to.gl_stat = from->gl_stat;
      to.gl_lstat = from->gl_lstat;
    }
  return to;
}

/* One component of a directory pattern, and what has to happen to the
   directories it matches.  */
struct glob_dir_step
{
  /* Component to match in every directory found so far.  */
  const char *pattern;
  /* Append a slash to the directories this step leaves behind.  */
  bool mark;
};

static size_t
glob_count (const char *s, char c)
{
  size_t n = 0;
  const char *p = strchr (s, c);

  while (p != NULL)
    {
      ++n;
      p = strchr (p + 1, c);
    }
  return n;
}

/* Expand DIRPATTERN, the directory part of a pattern, into PGLOB.  FLAGS
   holds GLOB_ERR, GLOB_NOESCAPE and GLOB_ALTDIRFUNC as inherited from the
   caller, plus GLOB_NOSORT and GLOB_ONLYDIR.

   DIRPATTERN is split at its slashes into components, each recorded in a
   heap-allocated array along with what it has to do with the directories
   it matches.  The components are then matched from left to right,
   starting at the longest leading part that has no metacharacter.  Stack
   usage does not depend on how many components there are, and a component
   that matches nothing ends the expansion at once.  */
static int
glob_dir_pattern (const char *dirpattern, int flags,
                  int (*errfunc) (const char *, int), glob_t *pglob)
{
  bool quote = !(flags & GLOB_NOESCAPE);
  struct glob_dir_step *steps = NULL;
  size_t nsteps = 0;
  size_t maxsteps;
  char *work;
  char *p;
  const char *base;
  const char *base_pattern;
  size_t baselen;
  /* Whether the level being scanned appends slashes to its result.  A
     component followed by a slash passes this down instead of matching
     anything itself, exactly as GLOB_MARK is passed down by glob.  */
  bool mark = false;
  glob_t cur;
  int retval;

  /* The pattern is split in place, so work on a copy.  */
  work = strdup (dirpattern);
  if (work == NULL)
    return GLOB_NOSPACE;
  p = work;

  /* Every step splits the pattern at a slash, so there is never more than one
     step per slash.  */
  maxsteps = glob_count (dirpattern, '/');
  if (maxsteps != 0)
    {
      steps = reallocarray (NULL, maxsteps, sizeof *steps);
      if (steps == NULL)
        {
          free (work);
          return GLOB_NOSPACE;
        }
    }

  pglob->gl_pathc = 0;
  pglob->gl_pathv = NULL;
  pglob->gl_offs = 0;

  cur = glob_copy_dirfuncs (pglob, flags);

  /* Split off the trailing components until what is left has no
     metacharacter; that is the directory the expansion starts from.  */
  for (;;)
    {
      char *filename = strrchr (p, '/');
      size_t dirlen;
      int meta;

      if (filename == NULL)
        {
          if (__glibc_unlikely (p[0] == '\0'))
            {
              retval = GLOB_NOMATCH;
              goto out;
            }
          base = ".";
          baselen = 0;
          base_pattern = p;
          break;
        }

      if (filename == p || (quote && filename == p + 1 && p[0] == '\\'))
        {
          /* "/pattern" or "\\/pattern".  */
          base = "/";
          baselen = 1;
          base_pattern = filename + 1;
          break;
        }

      dirlen = filename - p;
      *filename++ = '\0';

      if (filename[0] == '\0' && dirlen > 1)
        {
          /* "pattern/".  Expand "pattern", appending slashes.  */
          if (quote && p[dirlen - 1] == '\\')
            dirlen = glob_strip_backslash (p, dirlen);
          /* Collapse a run of trailing slashes so that the number of
	     components does not grow with the number of slashes.  */
          for (char *q = &p[dirlen - 1];
               q > p && q[0] == '/' && q[-1] == '/'; --q)
            q[0] = '\0';
          mark = true;
          continue;
        }

      meta = __glob_pattern_type (p, quote);
      if (meta & (GLOBPAT_SPECIAL | GLOBPAT_BRACKET))
        {
          if (quote && dirlen > 0 && p[dirlen - 1] == '\\')
            dirlen = glob_strip_backslash (p, dirlen);
          assert (nsteps < maxsteps);
          steps[nsteps].pattern = filename;
          steps[nsteps].mark = mark;
          ++nsteps;
          /* glob drops GLOB_MARK when it recurses on the directory
             part, so this level's marking does not carry further
             down.  */
          mark = false;
          continue;
        }

      if (meta & GLOBPAT_BACKSLASH)
        dirlen = glob_unescape (p, dirlen);
      base = p;
      baselen = dirlen;
      base_pattern = filename;
      break;
    }

  /* Now match the components, left to right.  */
  retval = glob_in_dir (base_pattern, base, flags, errfunc, &cur);
  if (retval != 0)
    goto out;
  if (baselen > 0 && prefix_array (base, cur.gl_pathv, cur.gl_pathc))
    {
      retval = GLOB_NOSPACE;
      goto out;
    }
  if (mark && !glob_mark_dirs (&cur, 0, flags))
    {
      retval = GLOB_NOSPACE;
      goto out;
    }

  for (size_t i = nsteps; i-- > 0; )
    {
      glob_t next = glob_copy_dirfuncs (pglob, flags);

      for (size_t j = 0; j < cur.gl_pathc; ++j)
        {
          size_t old = next.gl_pathc;
          int status = glob_in_dir (steps[i].pattern, cur.gl_pathv[j],
                                    flags | GLOB_APPEND, errfunc, &next);
          if (status == GLOB_NOMATCH)
            /* No matches in this directory.  Try the next.  */
            continue;
          if (status != 0)
            {
              globfree (&next);
              retval = status;
              goto out;
            }

          /* Stick the directory on the front of each name.  */
          if (prefix_array (cur.gl_pathv[j], &next.gl_pathv[old],
                            next.gl_pathc - old))
            {
              globfree (&next);
              retval = GLOB_NOSPACE;
              goto out;
            }
        }

      globfree (&cur);
      cur = next;

      if (cur.gl_pathc == 0)
        {
          retval = GLOB_NOMATCH;
          goto out;
        }

      if (steps[i].mark && !glob_mark_dirs (&cur, 0, flags))
        {
          retval = GLOB_NOSPACE;
          goto out;
        }
    }

  pglob->gl_pathc = cur.gl_pathc;
  pglob->gl_pathv = cur.gl_pathv;
  cur.gl_pathv = NULL;
  retval = 0;

 out:
  globfree (&cur);
  free (steps);
  free (work);
  return retval;
}

/* Return the first '{' in PATTERN that is not quoted, or NULL.  */
static const char *
glob_find_brace (const char *pattern, int flags)
{
  const char *p;

  if (flags & GLOB_NOESCAPE)
    return strchr (pattern, '{');

  for (p = pattern; *p != '\0'; ++p)
    {
      if (*p == '\\' && p[1] != '\0')
        ++p;
      else if (*p == '{')
        return p;
    }
  return NULL;
}

/* BEGIN points at the '{' of a brace expression.  Set *NEXT to the terminator
   of its first alternative and *REST to the text following the matching '}'.
   Return false if the expression is malformed.  */
static bool
glob_parse_brace (const char *begin, int flags, const char **next,
                  const char **rest)
{
  const char *n = next_brace_sub (begin + 1, flags);
  const char *r;

  if (n == NULL)
    return false;

  /* Find the end of the whole brace expression.  */
  for (r = n; *r != '}'; )
    {
      r = next_brace_sub (r + 1, flags);
      if (r == NULL)
        return false;
    }

  *next = n;
  *rest = r + 1;
  return true;
}

/* One brace expression being expanded.  P, NEXT and REST point into the
   string this level was created from, which is the enclosing level's BUF, or
   the pattern itself for the outermost level.  */
struct glob_brace_level
{
  char *buf;             /* This level's expansion.  */
  char *alt_start;       /* Where the alternative goes in BUF.  */
  const char *p;         /* Start of the current alternative.  */
  const char *next;      /* Its terminating ',' or '}'.  */
  const char *rest;      /* Text following the closing '}'.  */
  size_t rest_len;       /* strlen (rest) + 1.  */
  bool last;             /* The current alternative is the last one.  */
};

/* Start expanding the brace expression at BEGIN in S, pushing a level onto
   STACK.  Return false if the expression is malformed, which is not an error
   (the caller globs S as it stands).  Set *NOSPACE and return false if memory
   could not be allocated.  */
static bool
glob_brace_push (struct glob_brace_level *stack, size_t *depth,
                 const char *s, const char *begin, int flags, bool *nospace)
{
  struct glob_brace_level *level;
  const char *next;
  const char *rest;
  char *buf;

  *nospace = false;

  if (!glob_parse_brace (begin, flags, &next, &rest))
    return false;

  /* An expansion is at least the opening and closing brace shorter than the
     string it comes from.  */
  buf = malloc (strlen (s) - 1);
  if (buf == NULL)
    {
      *nospace = true;
      return false;
    }

  level = &stack[*depth];
  level->buf = buf;
  /* The prefix is the same for every alternative.  */
  level->alt_start = mempcpy (buf, s, begin - s);
  level->p = begin + 1;
  level->next = next;
  level->rest = rest;
  level->rest_len = strlen (rest) + 1;
  level->last = false;
  ++*depth;
  return true;
}

enum
{
  GLOB_BRACE_MATCHED,
  GLOB_BRACE_UNMATCHED,
  GLOB_BRACE_MALFORMED
};

/* Expand the brace expression at BEGIN in PATTERN, and any brace expression
   an expansion of it contains, and glob each result into PGLOB with
   GLOB_APPEND.  Store into *ACTION what the caller still has to do, the
   return value is meaningful only for GLOB_BRACE_MATCHED.

   The expansions are enumerated with an explicit, heap-allocated stack.  */
static int
glob_brace (const char *pattern, const char *begin, int flags,
            int (*errfunc) (const char *, int), glob_t *pglob, int *action)
{
  struct glob_brace_level *stack;
  size_t depth = 0;
  /* Every level consumes the leftmost brace of the string it expands, so the
     nesting can never exceed the number of braces in PATTERN, and the stack
     can be sized up front.  */
  size_t maxdepth = glob_count (pattern, '{');
  size_t firstc = pglob->gl_pathc;
  int subflags = (flags & ~(GLOB_NOCHECK | GLOB_NOMAGIC)) | GLOB_APPEND;
  int retval = 0;
  int status;
  bool nospace;

  *action = GLOB_BRACE_MATCHED;

  stack = reallocarray (NULL, maxdepth, sizeof *stack);
  if (stack == NULL)
    return GLOB_NOSPACE;

  if (!glob_brace_push (stack, &depth, pattern, begin, flags, &nospace))
    {
      free (stack);
      if (nospace)
        return GLOB_NOSPACE;
      *action = GLOB_BRACE_MALFORMED;
      return 0;
    }

  while (depth > 0)
    {
      struct glob_brace_level *level = &stack[depth - 1];
      const char *nested;
      char *expansion;

      if (level->last)
        {
          /* Every alternative of this expression has been produced.  */
          free (level->buf);
          --depth;
          continue;
        }

      /* Construct the expansion for the current alternative.  */
      mempcpy (mempcpy (level->alt_start, level->p, level->next - level->p),
               level->rest, level->rest_len);

      /* Step to the next alternative for when this level comes up again.  */
      if (*level->next == '}')
        level->last = true;
      else
        {
          level->p = level->next + 1;
          level->next = next_brace_sub (level->p, flags);
          assert (level->next != NULL);
        }

      /* If the expansion still holds a brace expression, expand that one
	 before globbing.  */
      expansion = level->buf;
      nested = glob_find_brace (expansion, flags);
      if (nested != NULL)
        {
          assert (depth < maxdepth);
          if (glob_brace_push (stack, &depth, expansion, nested, flags,
                               &nospace))
            continue;
          if (nospace)
            {
              retval = GLOB_NOSPACE;
              goto out;
            }
          /* Malformed: glob the expansion as it stands.  */
        }

      status = glob_internal (expansion, subflags, errfunc, pglob);
      if (status != 0 && status != GLOB_NOMATCH)
        {
          retval = status;
          goto out;
        }
    }

 out:
  while (depth > 0)
    free (stack[--depth].buf);
  free (stack);

  if (retval != 0)
    {
      if (!(flags & GLOB_APPEND))
        {
          globfree (pglob);
          pglob->gl_pathc = 0;
        }
      return retval;
    }

  if (pglob->gl_pathc != firstc)
    return 0;
  if (!(flags & (GLOB_NOCHECK | GLOB_NOMAGIC)))
    return GLOB_NOMATCH;

  *action = GLOB_BRACE_UNMATCHED;
  return 0;
}

/* Worker for __glob.  */
static int
GLOB_ATTRIBUTE
glob_internal (const char *pattern, int flags,
               int (*errfunc) (const char *, int), glob_t *pglob)
{
  const char *filename;
  char *dirname = NULL;
  size_t dirlen;
  int status;
  size_t oldcount;
  int meta;
  int dirname_modified;
  int malloc_dirname = 0;
  glob_t dirs;
  int retval = 0;

  if (pattern == NULL || pglob == NULL || (flags & ~__GLOB_FLAGS) != 0)
    {
      __set_errno (EINVAL);
      return -1;
    }

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

          if (pglob->gl_offs >= ~((size_t) 0) / sizeof (char *))
            return GLOB_NOSPACE;

          pglob->gl_pathv = (char **) malloc ((pglob->gl_offs + 1)
                                              * sizeof (char *));
          if (pglob->gl_pathv == NULL)
            return GLOB_NOSPACE;

          for (i = 0; i <= pglob->gl_offs; ++i)
            pglob->gl_pathv[i] = NULL;
        }
    }

  if (flags & GLOB_BRACE)
    {
      const char *begin = glob_find_brace (pattern, flags);

      if (begin != NULL)
        {
          int action;
          int result = glob_brace (pattern, begin, flags, errfunc, pglob,
                                   &action);

          if (action == GLOB_BRACE_MATCHED)
            return result;
          if (action == GLOB_BRACE_MALFORMED)
            flags &= ~GLOB_BRACE;
          /* Otherwise the expansion matched nothing and GLOB_NOCHECK or
             GLOB_NOMAGIC asks for the pattern itself to be globbed.  */
        }
    }

  oldcount = pglob->gl_pathc + pglob->gl_offs;

  /* Find the filename.  */
  filename = strrchr (pattern, '/');

#if defined __MSDOS__ || defined WINDOWS32
  /* The case of "d:pattern".  Since ':' is not allowed in
     file names, we can safely assume that wherever it
     happens in pattern, it signals the filename part.  This
     is so we could some day support patterns like "[a-z]:foo".  */
  if (filename == NULL)
    filename = strchr (pattern, ':');
#endif /* __MSDOS__ || WINDOWS32 */

  dirname_modified = 0;
  if (filename == NULL)
    {
      /* This can mean two things: a simple name or "~name".  The latter
         case is nothing but a notation for a directory.  */
      if ((flags & (GLOB_TILDE|GLOB_TILDE_CHECK)) && pattern[0] == '~')
        {
          dirname = (char *) pattern;
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
          dirname = (char *) ".";
          dirlen = 0;
        }
    }
  else if (filename == pattern
           || (filename == pattern + 1 && pattern[0] == '\\'
               && (flags & GLOB_NOESCAPE) == 0))
    {
      /* "/pattern" or "\\/pattern".  */
      dirname = (char *) "/";
      dirlen = 1;
      ++filename;
    }
  else
    {
      char *newp;
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
            return GLOB_NOMATCH;
          /* If this is "d:pattern", we need to copy ':' to DIRNAME
             as well.  If it's "d:/pattern", don't remove the slash
             from "d:/", since "d:" and "d:/" are not the same.*/
        }
#endif

      newp = malloc (dirlen + 1);
      if (newp == NULL)
        return GLOB_NOSPACE;
      malloc_dirname = 1;
      *((char *) mempcpy (newp, pattern, dirlen)) = '\0';
      dirname = newp;
      ++filename;

#if defined __MSDOS__ || defined WINDOWS32
      bool drive_root = (dirlen > 1
                         && (dirname[dirlen - 1] == ':'
                             || (dirlen > 2 && dirname[dirlen - 2] == ':'
                                 && dirname[dirlen - 1] == '/')));
#else
      bool drive_root = false;
#endif

      if (filename[0] == '\0' && dirlen > 1 && !drive_root)
        /* "pattern/".  Expand "pattern", appending slashes.  */
        {
          int orig_flags = flags;
          if (!(flags & GLOB_NOESCAPE) && dirname[dirlen - 1] == '\\')
            {
              /* "pattern\\/".  Remove the final backslash if it hasn't
                 been quoted.  */
              char *p = (char *) &dirname[dirlen - 1];

              while (p > dirname && p[-1] == '\\') --p;
              if ((&dirname[dirlen] - p) & 1)
                {
                  *(char *) &dirname[--dirlen] = '\0';
                  flags &= ~(GLOB_NOCHECK | GLOB_NOMAGIC);
                }
            }
          /* Prevent stack overflows with many trailing '/' characters.  */
          for (char *p = &dirname[dirlen - 1];
               p > dirname && p[0] == '/' && p[-1] == '/'; --p)
            p[0] = '\0';
          int val = glob_internal (dirname, flags | GLOB_MARK, errfunc,
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

  if ((flags & (GLOB_TILDE|GLOB_TILDE_CHECK)) && dirname[0] == '~')
    {
      if (dirname[1] == '\0' || dirname[1] == '/'
          || (!(flags & GLOB_NOESCAPE) && dirname[1] == '\\'
              && (dirname[2] == '\0' || dirname[2] == '/')))
        {
          /* Look up home directory.  */
          char *home_dir = getenv ("HOME");
          int malloc_home_dir = 0;
          if (home_dir == NULL || home_dir[0] == '\0')
            {
#ifdef WINDOWS32
              /* Windows NT defines HOMEDRIVE and HOMEPATH.  But give
                 preference to HOME, because the user can change HOME.  */
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
#else
              bool nospace;

              home_dir = glob_current_home_dir (&nospace);
              if (home_dir != NULL)
                malloc_home_dir = 1;
              else if (nospace)
                {
                  retval = GLOB_NOSPACE;
                  goto out;
                }
#endif /* WINDOWS32 */
            }
          if (home_dir == NULL || home_dir[0] == '\0')
            {
              if (__glibc_unlikely (malloc_home_dir))
                free (home_dir);
              if (flags & GLOB_TILDE_CHECK)
                {
                  retval = GLOB_NOMATCH;
                  goto out;
                }
              else
                {
                  home_dir = (char *) "~"; /* No luck.  */
                  malloc_home_dir = 0;
                }
            }
          /* Now construct the full directory.  */
          if (dirname[1] == '\0')
            {
              if (__glibc_unlikely (malloc_dirname))
                free (dirname);

              dirname = home_dir;
              dirlen = strlen (dirname);
              malloc_dirname = malloc_home_dir;
            }
          else
            {
              char *newp;
              size_t home_len = strlen (home_dir);

              newp = malloc (home_len + dirlen);
              if (newp == NULL)
                {
                  if (__glibc_unlikely (malloc_home_dir))
                    free (home_dir);
                  retval = GLOB_NOSPACE;
                  goto out;
                }

              mempcpy (mempcpy (newp, home_dir, home_len),
                       &dirname[1], dirlen);

              if (__glibc_unlikely (malloc_dirname))
                free (dirname);

              dirname = newp;
              dirlen += home_len - 1;
              malloc_dirname = 1;

              if (__glibc_unlikely (malloc_home_dir))
                free (home_dir);
            }
          dirname_modified = 1;
        }
      else
        {
#ifndef WINDOWS32
          /* Recognize ~user as a shorthand for the specified user's home
             directory.  */
          char *end_name = strchr (dirname, '/');
          char *user_name;
          int malloc_user_name = 0;
          char *unescape = NULL;

          if (!(flags & GLOB_NOESCAPE))
            {
              if (end_name == NULL)
                {
                  unescape = strchr (dirname, '\\');
                  if (unescape)
                    end_name = strchr (unescape, '\0');
                }
              else
                unescape = memchr (dirname, '\\', end_name - dirname);
            }
          if (end_name == NULL)
            user_name = dirname + 1;
          else
            {
              char *newp = malloc (end_name - dirname);
              if (newp == NULL)
                {
                  retval = GLOB_NOSPACE;
                  goto out;
                }
              malloc_user_name = 1;
              if (unescape != NULL)
                {
                  char *p = mempcpy (newp, dirname + 1,
                                     unescape - dirname - 1);
                  char *q = unescape;
                  while (q != end_name)
                    {
                      if (*q == '\\')
                        {
                          if (q + 1 == end_name)
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
                *((char *) mempcpy (newp, dirname + 1, end_name - dirname - 1))
                  = '\0';
              user_name = newp;
            }

          /* Look up specific user's home directory.  */
          {
            bool nospace;
            char *home_dir = glob_user_home_dir (user_name, &nospace);

            if (__glibc_unlikely (malloc_user_name))
              free (user_name);

            if (__glibc_unlikely (nospace))
              {
                retval = GLOB_NOSPACE;
                goto out;
              }

            /* If we found a home directory use this.  */
            if (home_dir != NULL)
              {
                size_t home_len = strlen (home_dir);
                size_t rest_len = end_name == NULL ? 0 : strlen (end_name);
                /* dirname contains end_name; we can't free it now.  */
                char *prev_dirname =
                  (__glibc_unlikely (malloc_dirname) ? dirname : NULL);
                char *d;

                malloc_dirname = 0;

                dirname = malloc (home_len + rest_len + 1);
                if (dirname == NULL)
                  {
                    free (prev_dirname);
                    free (home_dir);
                    retval = GLOB_NOSPACE;
                    goto out;
                  }
                malloc_dirname = 1;
                d = mempcpy (dirname, home_dir, home_len);
                if (end_name != NULL)
                  d = mempcpy (d, end_name, rest_len);
                *d = '\0';

                free (prev_dirname);
                free (home_dir);

                dirlen = home_len + rest_len;
                dirname_modified = 1;
              }
            else
              {
                if (flags & GLOB_TILDE_CHECK)
                  {
                    /* We have to regard it as an error if we cannot find the
                       home directory.  */
                    retval = GLOB_NOMATCH;
                    goto out;
                  }
              }
          }
#else /* WINDOWS32 */
          /* On native Windows, access to a user's home directory
             (via GetUserProfileDirectory) or to a user's environment
             variables (via ExpandEnvironmentStringsForUser) requires
             the credentials of the user.  Therefore we cannot support
             the ~user syntax on this platform.
             Handling ~user specially (and treat it like plain ~) if
             user is getenv ("USERNAME") would not be a good idea,
             since it would make people think that ~user is supported
             in general.  */
          if (flags & GLOB_TILDE_CHECK)
            {
              retval = GLOB_NOMATCH;
              goto out;
            }
#endif /* WINDOWS32 */
        }
    }

  /* Now test whether we looked for "~" or "~NAME".  In this case we
     can give the answer now.  */
  if (filename == NULL)
    {
      size_t newcount = pglob->gl_pathc + pglob->gl_offs;
      char **new_gl_pathv;

      if (newcount > SIZE_MAX / sizeof (char *) - 2)
        {
        nospace:
          free (pglob->gl_pathv);
          pglob->gl_pathv = NULL;
          pglob->gl_pathc = 0;
          retval = GLOB_NOSPACE;
          goto out;
        }

      new_gl_pathv = realloc (pglob->gl_pathv,
                              (newcount + 2) * sizeof (char *));
      if (new_gl_pathv == NULL)
        goto nospace;
      pglob->gl_pathv = new_gl_pathv;

      if (flags & GLOB_MARK && is_dir (dirname, flags, pglob))
        {
          char *p;
          pglob->gl_pathv[newcount] = malloc (dirlen + 2);
          if (pglob->gl_pathv[newcount] == NULL)
            goto nospace;
          p = mempcpy (pglob->gl_pathv[newcount], dirname, dirlen);
          p[0] = '/';
          p[1] = '\0';
          if (__glibc_unlikely (malloc_dirname))
            free (dirname);
        }
      else
        {
          if (__glibc_unlikely (malloc_dirname))
            pglob->gl_pathv[newcount] = dirname;
          else
            {
              pglob->gl_pathv[newcount] = strdup (dirname);
              if (pglob->gl_pathv[newcount] == NULL)
                goto nospace;
            }
        }
      pglob->gl_pathv[++newcount] = NULL;
      ++pglob->gl_pathc;
      pglob->gl_flags = flags;

      return 0;
    }

  meta = __glob_pattern_type (dirname, !(flags & GLOB_NOESCAPE));
  /* meta is 1 if correct glob pattern containing metacharacters.
     If meta has bit (1 << 2) set, it means there was an unterminated
     [ which we handle the same, using fnmatch.  Broken unterminated
     pattern bracket expressions ought to be rare enough that it is
     not worth special casing them, fnmatch will do the right thing.  */
  if (meta & (GLOBPAT_SPECIAL | GLOBPAT_BRACKET))
    {
      /* The directory name contains metacharacters, so we
         have to glob for the directory, and then glob for
         the pattern in each directory found.  */
      size_t i;

      if (!(flags & GLOB_NOESCAPE) && dirlen > 0 && dirname[dirlen - 1] == '\\')
        {
          /* "foo\\/bar".  Remove the final backslash from dirname
             if it has not been quoted.  */
          char *p = (char *) &dirname[dirlen - 1];

          while (p > dirname && p[-1] == '\\') --p;
          if ((&dirname[dirlen] - p) & 1)
            *(char *) &dirname[--dirlen] = '\0';
        }

      dirs = glob_copy_dirfuncs (pglob, flags);

      status = glob_dir_pattern (dirname,
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
              retval = GLOB_NOSPACE;
              goto out;
            }
        }

      flags |= GLOB_MAGCHAR;

      /* We have ignored the GLOB_NOCHECK flag in the 'glob_in_dir' calls.
         But if we have not found any matching entry and the GLOB_NOCHECK
         flag was set we must return the input pattern itself.  */
      if (pglob->gl_pathc + pglob->gl_offs == oldcount)
        {
        no_matches:
          /* No matches.  */
          if (flags & GLOB_NOCHECK)
            {
              size_t newcount = pglob->gl_pathc + pglob->gl_offs;
              char **new_gl_pathv;

              if (newcount > SIZE_MAX / sizeof (char *) - 2)
                {
                nospace2:
                  globfree (&dirs);
                  retval = GLOB_NOSPACE;
                  goto out;
                }

              new_gl_pathv = realloc (pglob->gl_pathv,
                                      (newcount + 2) * sizeof (char *));
              if (new_gl_pathv == NULL)
                goto nospace2;
              pglob->gl_pathv = new_gl_pathv;

              pglob->gl_pathv[newcount] = strdup (pattern);
              if (pglob->gl_pathv[newcount] == NULL)
                {
                  globfree (&dirs);
                  globfree (pglob);
                  pglob->gl_pathc = 0;
                  retval = GLOB_NOSPACE;
                  goto out;
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

      if (meta & GLOBPAT_BACKSLASH)
        {
          /* We need to unescape the dirname string.  It is certainly
             our own copy, as otherwise filename would be NULL or
             dirname wouldn't contain backslashes.  */
          dirlen = glob_unescape (dirname, dirlen);
          dirname_modified = 1;
        }
      if (dirname_modified)
        flags &= ~(GLOB_NOCHECK | GLOB_NOMAGIC);
      status = glob_in_dir (filename, dirname, flags, errfunc, pglob);
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
          if (prefix_array (dirname,
                            &pglob->gl_pathv[old_pathc + pglob->gl_offs],
                            pglob->gl_pathc - old_pathc))
            {
              globfree (pglob);
              pglob->gl_pathc = 0;
              retval = GLOB_NOSPACE;
              goto out;
            }
        }
    }

  if ((flags & GLOB_MARK) && !glob_mark_dirs (pglob, oldcount, flags))
    {
      globfree (pglob);
      pglob->gl_pathc = 0;
      retval = GLOB_NOSPACE;
      goto out;
    }

  if (!(flags & GLOB_NOSORT))
    {
      /* Sort the vector.  */
      qsort (&pglob->gl_pathv[oldcount],
             pglob->gl_pathc + pglob->gl_offs - oldcount,
             sizeof (char *), collated_compare);
    }

 out:
  if (__glibc_unlikely (malloc_dirname))
    free (dirname);

  return retval;
}
#if defined _LIBC && !defined __glob
versioned_symbol (libc, __glob, glob, GLIBC_2_27);
libc_hidden_ver (__glob, glob)
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
  char dirsep_char = '/';

  if (dirlen == 1 && dirname[0] == '/')
    /* DIRNAME is just "/", so normal prepending would get us "//foo".
       We want "/foo" instead, so don't prepend any chars from DIRNAME.  */
    dirlen = 0;

#if defined __MSDOS__ || defined WINDOWS32
  if (dirlen > 1)
    {
      if (dirname[dirlen - 1] == '/' && dirname[dirlen - 2] == ':')
        /* DIRNAME is "d:/".  Don't prepend the slash from DIRNAME.  */
        --dirlen;
      else if (dirname[dirlen - 1] == ':')
        {
          /* DIRNAME is "d:".  Use ':' instead of '/'.  */
          --dirlen;
          dirsep_char = ':';
        }
    }
#endif

  for (i = 0; i < n; ++i)
    {
      size_t eltlen = strlen (array[i]) + 1;
      char *new = malloc (dirlen + 1 + eltlen);
      if (new == NULL)
        {
          while (i > 0)
            free (array[--i]);
          return 1;
        }

      {
        char *endp = mempcpy (new, dirname, dirlen);
        *endp++ = dirsep_char;
        mempcpy (endp, array[i], eltlen);
      }
      free (array[i]);
      array[i] = new;
    }

  return 0;
}

/* Like 'glob', but PATTERN is a final pathname component,
   and matches are searched for in DIRECTORY.
   The GLOB_NOSORT bit in FLAGS is ignored.  No sorting is ever done.
   The GLOB_APPEND flag is assumed to be set (always appends).  */
static int
glob_in_dir (const char *pattern, const char *directory, int flags,
             int (*errfunc) (const char *, int), glob_t *pglob)
{
  size_t dirlen = strlen (directory);
  void *stream = NULL;
  struct scratch_buffer s;
  scratch_buffer_init (&s);
# define GLOBNAMES_MEMBERS(nnames) \
    struct globnames *next; size_t count; char *name[nnames];
  struct globnames { GLOBNAMES_MEMBERS (FLEXIBLE_ARRAY_MEMBER) };
  struct { GLOBNAMES_MEMBERS (64) } init_names_buf;
  struct globnames *init_names = (struct globnames *) &init_names_buf;
  struct globnames *names = init_names;
  size_t nfound = 0;
  size_t cur = 0;
  int meta;
  int save;
  int result;

  init_names->next = NULL;
  init_names->count = ((sizeof init_names_buf
                        - offsetof (struct globnames, name))
                       / sizeof init_names->name[0]);

  meta = __glob_pattern_type (pattern, !(flags & GLOB_NOESCAPE));
  if (meta == GLOBPAT_NONE && (flags & (GLOB_NOCHECK|GLOB_NOMAGIC)))
    {
      /* We need not do any tests.  The PATTERN contains no meta
         characters and we must not return an error therefore the
         result will always contain exactly one name.  */
      flags |= GLOB_NOCHECK;
    }
  else if (meta == GLOBPAT_NONE)
    {
      size_t patlen = strlen (pattern);
      size_t fullsize;
      char *fullname;

      if (size_add_wrapv (dirlen + 1, patlen + 1, &fullsize))
        return GLOB_NOSPACE;
      fullname = malloc (fullsize);
      if (fullname == NULL)
        return GLOB_NOSPACE;

      mempcpy (mempcpy (mempcpy (fullname, directory, dirlen),
                        "/", 1),
               pattern, patlen + 1);
      if (glob_lstat (pglob, flags, fullname) == 0
          || errno == EOVERFLOW)
        /* We found this file to be existing.  Now tell the rest
           of the function to copy this name into the result.  */
        flags |= GLOB_NOCHECK;

      free (fullname);
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
          int dfd = dirfd (stream);
          int fnm_flags = ((!(flags & GLOB_PERIOD) ? FNM_PERIOD : 0)
                           | ((flags & GLOB_NOESCAPE) ? FNM_NOESCAPE : 0));
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

              /* If we shall match only directories use the information
                 provided by the dirent call if possible.  */
              if (flags & GLOB_ONLYDIR)
                switch (readdir_result_type (d))
                  {
                  default: continue;
                  case DT_DIR: break;
                  case DT_LNK: case DT_UNKNOWN:
                    /* The filesystem was too lazy to give us a hint,
                       so we have to do it the hard way.  */
                    if (__glibc_unlikely (dfd < 0 || flags & GLOB_ALTDIRFUNC))
                      {
                        size_t namelen = strlen (d.name);
                        size_t need = dirlen + 1 + namelen + 1;
                        if (s.length < need
                            && !scratch_buffer_set_array_size (&s, need, 1))
                          goto memory_error;
                        char *p = mempcpy (s.data, directory, dirlen);
                        *p = '/';
                        p += p[-1] != '/';
                        memcpy (p, d.name, namelen + 1);
                        if (! is_dir (s.data, flags, pglob))
                          continue;
                      }
                    else
                      {
                        struct_stat64 st64;
                        if (! (GLOB_FSTATAT64 (dfd, d.name, &st64, 0) == 0
                               && S_ISDIR (st64.st_mode)))
                          continue;
                      }
                  }

              if (fnmatch (pattern, d.name, fnm_flags) == 0)
                {
                  if (cur == names->count)
                    {
                      struct globnames *newnames;
                      size_t count = names->count * 2;
                      size_t nameoff = offsetof (struct globnames, name);
                      size_t size = FLEXSIZEOF (struct globnames, name,
                                                count * sizeof (char *));
                      if ((SIZE_MAX - nameoff) / 2 / sizeof (char *)
                          < names->count)
                        goto memory_error;
                      newnames = malloc (size);
                      if (newnames == NULL)
                        goto memory_error;
                      newnames->count = count;
                      newnames->next = names;
                      names = newnames;
                      cur = 0;
                    }
                  names->name[cur] = strdup (d.name);
                  if (names->name[cur] == NULL)
                    goto memory_error;
                  ++cur;
                  ++nfound;
                  if (SIZE_MAX - pglob->gl_offs <= nfound)
                    goto memory_error;
                }
            }
        }
    }

  if (nfound == 0 && (flags & GLOB_NOCHECK))
    {
      size_t len = strlen (pattern);
      nfound = 1;
      names->name[cur] = malloc (len + 1);
      if (names->name[cur] == NULL)
        goto memory_error;
      *((char *) mempcpy (names->name[cur++], pattern, len)) = '\0';
    }

  result = GLOB_NOMATCH;
  if (nfound != 0)
    {
      char **new_gl_pathv;
      result = 0;

      if (SIZE_MAX / sizeof (char *) - pglob->gl_pathc
          < pglob->gl_offs + nfound + 1)
        goto memory_error;

      new_gl_pathv
        = realloc (pglob->gl_pathv,
                   (pglob->gl_pathc + pglob->gl_offs + nfound + 1)
                    * sizeof (char *));

      if (new_gl_pathv == NULL)
        {
        memory_error:
          while (1)
            {
              struct globnames *old = names;
              for (size_t i = 0; i < cur; ++i)
                free (names->name[i]);
              names = names->next;
              /* NB: we will not leak memory here if we exit without
                 freeing the current block assigned to OLD.  At least
                 the very first block is always allocated on the stack
                 and this is the block assigned to OLD here.  */
              if (names == NULL)
                {
                  assert (old == init_names);
                  break;
                }
              cur = names->count;
              free (old);
            }
          result = GLOB_NOSPACE;
        }
      else
        {
          while (1)
            {
              struct globnames *old = names;
              for (size_t i = 0; i < cur; ++i)
                new_gl_pathv[pglob->gl_offs + pglob->gl_pathc++]
                  = names->name[i];
              names = names->next;
              /* NB: we will not leak memory here if we exit without
                 freeing the current block assigned to OLD.  At least
                 the very first block is always allocated on the stack
                 and this is the block assigned to OLD here.  */
              if (names == NULL)
                {
                  assert (old == init_names);
                  break;
                }
              cur = names->count;
              free (old);
            }

          pglob->gl_pathv = new_gl_pathv;

          pglob->gl_pathv[pglob->gl_offs + pglob->gl_pathc] = NULL;

          pglob->gl_flags = flags;
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

  scratch_buffer_free (&s);
  return result;
}
