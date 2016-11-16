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

#ifndef	_STRINGS_H
#define	_STRINGS_H	1

#include <features.h>
#define __need_size_t
#include <stddef.h>

/* Tell the caller that we provide correct C++ prototypes.  */
#if defined __cplusplus && __GNUC_PREREQ (4, 4)
# define __CORRECT_ISO_CPP_STRINGS_H_PROTO
#endif
#include <bits/const-covariance.h>

__BEGIN_DECLS

#if defined __USE_MISC || !defined __USE_XOPEN2K8
/* Compare N bytes of S1 and S2 (same as memcmp).  */
extern int bcmp (const void *__s1, const void *__s2, size_t __n)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Copy N bytes of SRC to DEST (like memmove, but args reversed).  */
extern void bcopy (const void *__src, void *__dest, size_t __n)
  __THROW __nonnull ((1, 2));

/* Set N bytes of S to 0.  */
extern void bzero (void *__s, size_t __n) __THROW __nonnull ((1));

/* Find the first occurrence of C in S (same as strchr).
   [C]    extern char *index (const char *s, int c);
   [C++]  extern char *index (char *s, int c);
          extern const char *index (const char *s, int c);  */
__CONST_COV_BUILTIN (index, __attribute_pure__ __nonnull ((1)),
                     char *, __s, int, __c);

/* Find the last occurrence of C in S (same as strrchr).
   [C]    extern char *rindex (const char *s, int c);
   [C++]  extern char *rindex (char *s, int c);
          extern const char *rindex (const char *s, int c);  */
__CONST_COV_BUILTIN (rindex, __attribute_pure__ __nonnull ((1)),
                     char *, __s, int, __c);
#endif

#if defined __USE_MISC || !defined __USE_XOPEN2K8 || defined __USE_XOPEN2K8XSI
/* Return the position of the first bit set in I, or 0 if none are set.
   The least-significant bit is position 1, the most-significant 32.  */
extern int ffs (int __i) __THROW __attribute_const__;
#endif

/* The following two functions are non-standard but necessary for non-32 bit
   platforms.  */
#ifdef	__USE_GNU
extern int ffsl (long int __l) __THROW __attribute_const__;
__extension__ extern int ffsll (long long int __ll)
     __THROW __attribute_const__;
#endif

/* Compare S1 and S2, ignoring case.  */
extern int strcasecmp (const char *__s1, const char *__s2)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Compare no more than N chars of S1 and S2, ignoring case.  */
extern int strncasecmp (const char *__s1, const char *__s2, size_t __n)
     __THROW __attribute_pure__ __nonnull ((1, 2));

#ifdef	__USE_XOPEN2K8
# include <xlocale.h>

/* Compare S1 and S2, ignoring case, using collation rules from LOC.  */
extern int strcasecmp_l (const char *__s1, const char *__s2, __locale_t __loc)
     __THROW __attribute_pure__ __nonnull ((1, 2, 3));

/* Compare no more than N chars of S1 and S2, ignoring case, using
   collation rules from LOC.  */
extern int strncasecmp_l (const char *__s1, const char *__s2,
			  size_t __n, __locale_t __loc)
     __THROW __attribute_pure__ __nonnull ((1, 2, 4));
#endif

__END_DECLS

#endif	/* strings.h  */
