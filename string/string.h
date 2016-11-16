/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
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

/*
 *	ISO C99 Standard: 7.21 String handling	<string.h>
 */

#ifndef	_STRING_H
#define	_STRING_H	1

#define __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION
#include <bits/libc-header-start.h>

__BEGIN_DECLS

/* Get size_t and NULL from <stddef.h>.  */
#define	__need_size_t
#define	__need_NULL
#include <stddef.h>

/* Tell the caller that we provide correct C++ prototypes.  */
#if defined __cplusplus && __GNUC_PREREQ (4, 4)
# define __CORRECT_ISO_CPP_STRING_H_PROTO
#endif
#include <bits/const-covariance.h>

__BEGIN_NAMESPACE_STD
/* Copy N bytes of SRC to DEST.  */
extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
		     size_t __n) __THROW __nonnull ((1, 2));
/* Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
extern void *memmove (void *__dest, const void *__src, size_t __n)
     __THROW __nonnull ((1, 2));
__END_NAMESPACE_STD

/* Copy no more than N bytes of SRC to DEST, stopping when C is found.
   Return the position in DEST one byte past where C was copied,
   or NULL if C was not found in the first N bytes of SRC.  */
#if defined __USE_MISC || defined __USE_XOPEN
extern void *memccpy (void *__restrict __dest, const void *__restrict __src,
		      int __c, size_t __n)
     __THROW __nonnull ((1, 2));
#endif /* Misc || X/Open.  */


__BEGIN_NAMESPACE_STD
/* Set N bytes of S to C.  */
extern void *memset (void *__s, int __c, size_t __n) __THROW __nonnull ((1));

/* Compare N bytes of S1 and S2.  */
extern int memcmp (const void *__s1, const void *__s2, size_t __n)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Search N bytes of S for C.
   [C]    extern void *memchr (const void *s, int c, size_t n);
   [C++]  extern void *memchr (void *s, int c, size_t n);
          extern const void *memchr (const void *s, int c, size_t n);  */
__CONST_COV_BUILTIN (memchr, __attribute_pure__ __nonnull ((1)),
                     void *, __s, int, __c, size_t, __n);
__END_NAMESPACE_STD

#ifdef __USE_GNU
/* Search in S for C.  This is similar to 'memchr' but there is no
   length limit.
   [C]    extern void *rawmemchr (const void *s, int c);
   [C++]  extern void *rawmemchr (void *s, int c);
          extern const void *rawmemchr (const void *s, int c);  */
__CONST_COV_PROTO (rawmemchr, __attribute_pure__ __nonnull ((1)),
                   void *, __s, int, __c);

/* Search N bytes of S for the final occurrence of C.
   [C]    extern void *memrchr (const void *s, int c, size_t n);
   [C++]  extern void *memrchr (void *s, int c, size_t n);
          extern const void *memrchr (const void *s, int c, size_t n);  */
__CONST_COV_PROTO (memrchr, __attribute_pure__ __nonnull ((1)),
                   void *, __s, int, __c, size_t, __n);
#endif

__BEGIN_NAMESPACE_STD
/* Copy SRC to DEST.  */
extern char *strcpy (char *__restrict __dest, const char *__restrict __src)
     __THROW __nonnull ((1, 2));
/* Copy no more than N characters of SRC to DEST.  */
extern char *strncpy (char *__restrict __dest,
		      const char *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));

/* Append SRC onto DEST.  */
extern char *strcat (char *__restrict __dest, const char *__restrict __src)
     __THROW __nonnull ((1, 2));
/* Append no more than N characters from SRC onto DEST.  */
extern char *strncat (char *__restrict __dest, const char *__restrict __src,
		      size_t __n) __THROW __nonnull ((1, 2));

/* Compare S1 and S2.  */
extern int strcmp (const char *__s1, const char *__s2)
     __THROW __attribute_pure__ __nonnull ((1, 2));
/* Compare N characters of S1 and S2.  */
extern int strncmp (const char *__s1, const char *__s2, size_t __n)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Compare the collated forms of S1 and S2.  */
extern int strcoll (const char *__s1, const char *__s2)
     __THROW __attribute_pure__ __nonnull ((1, 2));
/* Put a transformation of SRC into no more than N bytes of DEST.  */
extern size_t strxfrm (char *__restrict __dest,
		       const char *__restrict __src, size_t __n)
     __THROW __nonnull ((2));
__END_NAMESPACE_STD

#ifdef __USE_XOPEN2K8
# include <xlocale.h>

/* Compare the collated forms of S1 and S2, using sorting rules from L.  */
extern int strcoll_l (const char *__s1, const char *__s2, __locale_t __l)
     __THROW __attribute_pure__ __nonnull ((1, 2, 3));
/* Put a transformation of SRC into no more than N bytes of DEST,
   using sorting rules from L.  */
extern size_t strxfrm_l (char *__dest, const char *__src, size_t __n,
			 __locale_t __l) __THROW __nonnull ((2, 4));
#endif

#if (defined __USE_XOPEN_EXTENDED || defined __USE_XOPEN2K8	\
     || __GLIBC_USE (LIB_EXT2))
/* Duplicate S, returning an identical malloc'd string.  */
extern char *strdup (const char *__s)
     __THROW __attribute_malloc__ __nonnull ((1));
#endif

/* Return a malloc'd copy of at most N bytes of STRING.  The
   resultant string is terminated even if no null terminator
   appears before STRING[N].  */
#if defined __USE_XOPEN2K8 || __GLIBC_USE (LIB_EXT2)
extern char *strndup (const char *__string, size_t __n)
     __THROW __attribute_malloc__ __nonnull ((1));
#endif

#if defined __USE_GNU && defined __GNUC__
/* Duplicate S, returning an identical alloca'd string.  */
# define strdupa(s)							      \
  (__extension__							      \
    ({									      \
      const char *__old = (s);						      \
      size_t __len = strlen (__old) + 1;				      \
      char *__new = (char *) __builtin_alloca (__len);			      \
      (char *) memcpy (__new, __old, __len);				      \
    }))

/* Return an alloca'd copy of at most N bytes of string.  */
# define strndupa(s, n)							      \
  (__extension__							      \
    ({									      \
      const char *__old = (s);						      \
      size_t __len = strnlen (__old, (n));				      \
      char *__new = (char *) __builtin_alloca (__len + 1);		      \
      __new[__len] = '\0';						      \
      (char *) memcpy (__new, __old, __len);				      \
    }))
#endif

__BEGIN_NAMESPACE_STD
/* Find the first occurrence of C in S.
   [C]    extern char *strchr (const char *s, int c);
   [C++]  extern char *strchr (char *s, int c);
          extern const char *strchr (const char *s, int c);  */
__CONST_COV_BUILTIN (strchr, __attribute_pure__ __nonnull ((1)),
                   char *, __s, int, __c);

/* Find the last occurrence of C in S.
   [C]    extern char *strrchr (const char *s, int c);
   [C++]  extern char *strrchr (char *s, int c);
          extern const char *strrchr (const char *s, int c);  */
__CONST_COV_BUILTIN (strrchr, __attribute_pure__ __nonnull ((1)),
                     char *, __s, int, __c);

__END_NAMESPACE_STD

#ifdef __USE_GNU
/* This function is similar to `strchr'.  But it returns a pointer to
   the closing NUL byte in case C is not found in S.
   [C]    extern char *strchrnul (const char *s, int c);
   [C++]  extern char *strchrnul (char *s, int c);
          extern const char *strchrnul (const char *s, int c);  */
__CONST_COV_PROTO (strchrnul, __attribute_pure__ __nonnull ((1)),
                   char *, __s, int, __c);
#endif

__BEGIN_NAMESPACE_STD
/* Return the length of the initial segment of S which
   consists entirely of characters not in REJECT.  */
extern size_t strcspn (const char *__s, const char *__reject)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Return the length of the initial segment of S which
   consists entirely of characters in ACCEPT.  */
extern size_t strspn (const char *__s, const char *__accept)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Find the first occurrence in S of any character in ACCEPT.
   [C]   extern char *strpbrk (const char *s, const char *accept);
   [C++] extern char *strpbrk (char *s, const char *accept);
         extern const char *strpbrk (const char *s, const char *accept); */
__CONST_COV_BUILTIN (strpbrk, __attribute_pure__ __nonnull ((1, 2)),
                     char *, __s, const char *, __accept);

/* Find the first occurrence of NEEDLE in HAYSTACK.
   [C]   extern char *strstr (const char *haystack, const char *needle);
   [C++] extern char *strstr (char *haystack, const char *needle);
         extern const char *strstr (const char *haystack, const char *needle);
 */
__CONST_COV_BUILTIN (strstr, __attribute_pure__ __nonnull ((1, 2)),
                     char *, __haystack, const char *, __needle);

/* Divide S into tokens separated by characters in DELIM.  */
extern char *strtok (char *__restrict __s, const char *__restrict __delim)
     __THROW __nonnull ((2));
__END_NAMESPACE_STD

/* Divide S into tokens separated by characters in DELIM.  Information
   passed between calls are stored in SAVE_PTR.  */
extern char *__strtok_r (char *__restrict __s,
			 const char *__restrict __delim,
			 char **__restrict __save_ptr)
     __THROW __nonnull ((2, 3));
#ifdef __USE_POSIX
extern char *strtok_r (char *__restrict __s, const char *__restrict __delim,
		       char **__restrict __save_ptr)
     __THROW __nonnull ((2, 3));
#endif

#ifdef __USE_GNU
/* Similar to 'strstr', but ignores the case of both strings.
   [C]   extern char *strcasestr (const char *haystack, const char *needle);
   [C++] extern char *strcasestr (char *haystack, const char *needle);
         extern const char *strcasestr (const char *haystack,
                                        const char *needle);  */
__CONST_COV_PROTO (strcasestr, __attribute_pure__ __nonnull ((1, 2)),
                   char *, __haystack, const char *, __needle);

/* Find the first occurrence of NEEDLE in HAYSTACK.
   NEEDLE is NEEDLELEN bytes long;
   HAYSTACK is HAYSTACKLEN bytes long.
   [C]   extern void *memmem (const void *haystack, size_t haystacklen,
                              const void *needle, size_t needlelen);
   [C++] extern void *memmem (void *haystack, size_t haystacklen,
                              const void *needle, size_t needlelen);
         extern const void *memmem (const void *haystack, size_t haystacklen,
                                    const void *needle, size_t needlelen);  */
__CONST_COV_PROTO (memmem, __attribute_pure__ __nonnull ((1, 3)),
                   void *, __haystack, size_t, __haystacklen,
                   const void *, __needle, size_t, __needlelen);

/* Copy N bytes of SRC to DEST, return pointer to bytes after the
   last written byte.  */
extern void *__mempcpy (void *__restrict __dest,
			const void *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));
extern void *mempcpy (void *__restrict __dest,
		      const void *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));
#endif


__BEGIN_NAMESPACE_STD
/* Return the length of S.  */
extern size_t strlen (const char *__s)
     __THROW __attribute_pure__ __nonnull ((1));
__END_NAMESPACE_STD

#ifdef	__USE_XOPEN2K8
/* Find the length of STRING, but scan at most MAXLEN characters.
   If no '\0' terminator is found in that many characters, return MAXLEN.  */
extern size_t strnlen (const char *__string, size_t __maxlen)
     __THROW __attribute_pure__ __nonnull ((1));
#endif


__BEGIN_NAMESPACE_STD
/* Return a string describing the meaning of the `errno' code in ERRNUM.  */
extern char *strerror (int __errnum) __THROW;
__END_NAMESPACE_STD
#ifdef __USE_XOPEN2K
/* Reentrant version of `strerror'.
   There are 2 flavors of `strerror_r', GNU which returns the string
   and may or may not use the supplied temporary buffer and POSIX one
   which fills the string into the buffer.
   To use the POSIX version, -D_XOPEN_SOURCE=600 or -D_POSIX_C_SOURCE=200112L
   without -D_GNU_SOURCE is needed, otherwise the GNU version is
   preferred.  */
# if defined __USE_XOPEN2K && !defined __USE_GNU
/* Fill BUF with a string describing the meaning of the `errno' code in
   ERRNUM.  */
#  ifdef __REDIRECT_NTH
extern int __REDIRECT_NTH (strerror_r,
			   (int __errnum, char *__buf, size_t __buflen),
			   __xpg_strerror_r) __nonnull ((2));
#  else
extern int __xpg_strerror_r (int __errnum, char *__buf, size_t __buflen)
     __THROW __nonnull ((2));
#   define strerror_r __xpg_strerror_r
#  endif
# else
/* If a temporary buffer is required, at most BUFLEN bytes of BUF will be
   used.  */
extern char *strerror_r (int __errnum, char *__buf, size_t __buflen)
     __THROW __nonnull ((2)) __wur;
# endif
#endif

#ifdef __USE_XOPEN2K8
/* Translate error number to string according to the locale L.  */
extern char *strerror_l (int __errnum, __locale_t __l) __THROW;
#endif

#ifdef __USE_MISC
# include <strings.h>
#endif

#ifdef	__USE_MISC
/* Return the next DELIM-delimited token from *STRINGP,
   terminating it with a '\0', and update *STRINGP to point past it.  */
extern char *strsep (char **__restrict __stringp,
		     const char *__restrict __delim)
     __THROW __nonnull ((1, 2));
#endif

#ifdef	__USE_XOPEN2K8
/* Return a string describing the meaning of the signal number in SIG.  */
extern char *strsignal (int __sig) __THROW;

/* Copy SRC to DEST, returning the address of the terminating '\0' in DEST.  */
extern char *__stpcpy (char *__restrict __dest, const char *__restrict __src)
     __THROW __nonnull ((1, 2));
extern char *stpcpy (char *__restrict __dest, const char *__restrict __src)
     __THROW __nonnull ((1, 2));

/* Copy no more than N characters of SRC to DEST, returning the address of
   the last character written into DEST.  */
extern char *__stpncpy (char *__restrict __dest,
			const char *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));
extern char *stpncpy (char *__restrict __dest,
		      const char *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));
#endif

#ifdef	__USE_GNU
/* Compare S1 and S2 as strings holding name & indices/version numbers.  */
extern int strverscmp (const char *__s1, const char *__s2)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Sautee STRING briskly.  */
extern char *strfry (char *__string) __THROW __nonnull ((1));

/* Frobnicate N bytes of S.  */
extern void *memfrob (void *__s, size_t __n) __THROW __nonnull ((1));

# ifndef basename
/* Return the file name within directory of FILENAME.  We don't
   declare the function if the `basename' macro is available (defined
   in <libgen.h>) which makes the XPG version of this function
   available.
   [C]   char *basename (const char *filename);
   [C++] char *basename (char *filename);
         const char *basename (const char *filename);  */
__CONST_COV_PROTO (basename, __nonnull ((1)),
                   char *, __filename);
# endif
#endif


#if __GNUC_PREREQ (3,4)
# if defined __OPTIMIZE__ && !defined __OPTIMIZE_SIZE__ \
     && !defined __NO_INLINE__ && !defined __cplusplus
/* When using GNU CC we provide some optimized versions of selected
   functions from this header.  There are two kinds of optimizations:

   - machine-dependent optimizations, most probably using inline
     assembler code; these might be quite expensive since the code
     size can increase significantly.
     These optimizations are not used unless the symbol
	__USE_STRING_INLINES
     is defined before including this header.

   - machine-independent optimizations which do not increase the
     code size significantly and which optimize mainly situations
     where one or more arguments are compile-time constants.
     These optimizations are used always when the compiler is
     taught to optimize.

   One can inhibit all optimizations by defining __NO_STRING_INLINES.  */

/* Get the machine-dependent optimizations (if any).  */
#  include <bits/string.h>

/* These are generic optimizations which do not add too much inline code.  */
#  include <bits/string2.h>
# endif

# if __USE_FORTIFY_LEVEL > 0 && defined __fortify_function
/* Functions with security checks.  */
#  include <bits/string3.h>
# endif
#endif

#if defined __USE_GNU && defined __OPTIMIZE__ \
    && defined __extern_always_inline && __GNUC_PREREQ (3,2)
# if !defined _FORCE_INLINES && !defined _HAVE_STRING_ARCH_mempcpy

#define mempcpy(dest, src, n) __mempcpy_inline (dest, src, n)
#define __mempcpy(dest, src, n) __mempcpy_inline (dest, src, n)

__extern_always_inline void *
__mempcpy_inline (void *__restrict __dest,
		  const void *__restrict __src, size_t __n)
{
  return (char *) memcpy (__dest, __src, __n) + __n;
}

# endif
#endif

__END_DECLS

#endif /* string.h  */
