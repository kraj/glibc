/* Copyright (C) 2016 Free Software Foundation, Inc.
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

#ifndef _BITS_CONST_COVARIANCE_H
#define _BITS_CONST_COVARIANCE_H 1

#if !defined _STRING_H && !defined _STRINGS_H && !defined _WCHAR_H
# error "Never use <bits/const-covariance.h> directly; include <string.h>, <strings.h>, or <wchar.h> instead."
#endif

/* This header defines internal-use macros that expand a C prototype
   declaration like

       extern void *memchr (const void *, int, size_t) attrs;

   to a pair of C++ overloaded function declarations with improved
   const-correctness:

       extern void *memchr (void *, int, size_t) attrs;
       extern const void *memchr (const void *, int, size_t) attrs;

   You use them like this:

       __CONST_COV_PROTO (memchr, attrs,
                          void *, __s, int, __c, size_t, __n);

    where the arguments after 'attrs' are the function's arguments,
    alternating with argument names.  The first of these will be used
    as the const-covariant return type.  It should be written without
    a 'const' qualifier.

    If the compiler has intrinsic knowledge of the function, use
    __CONST_COV_BUILTIN instead of __CONST_COV_PROTO.  In C++ mode,
    this will also generate inline functions of the form

        __extern_always_inline [const] void *
        memchr (void *__s, int __c, size_t __n) attrs
        {
          return __builtin_memchr (__s, __c, __n);
        }

    Due to limitations in the preprocessor, these macros support no
    more than four arguments to any function.  This is all that
    string.h/strings.h currently require.

    Because g++ only accepts throw(), __asm("..."), and
    __attribute__((whatever)) annotations in a specific order, all
    functions declared with __CONST_COV_PROTO or defined with
    __CONST_COV_BUILTIN are automatically marked __THROW.  Do not put
    __THROW in 'attrs'.  */

#define __CC_VA_NARGS(...)  __CC_VA_NARGS_(__VA_ARGS__, __CC_RSEQ)
#define __CC_VA_NARGS_(...) __CC_VA_NARGS__(__VA_ARGS__)
#define __CC_VA_NARGS__(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define __CC_RSEQ 8, 7, 6, 5, 4, 3, 2, 1

#define __CC_2FOR2(op, a, b)	  op (a, b)
#define __CC_2FOR4(op, a, b, ...) op (a, b), __CC_2FOR2 (op, __VA_ARGS__)
#define __CC_2FOR6(op, a, b, ...) op (a, b), __CC_2FOR4 (op, __VA_ARGS__)
#define __CC_2FOR8(op, a, b, ...) op (a, b), __CC_2FOR6 (op, __VA_ARGS__)

#define __CC_2FOR(op, ...) \
 __CC_2FOR_ (__CC_VA_NARGS (__VA_ARGS__)) (op, __VA_ARGS__)
#define __CC_2FOR_(n) __CC_2FOR__ (n)
#define __CC_2FOR__(n) __CC_2FOR##n

#define __CC_XPROTO(...) (__CC_2FOR (__CC_XPROTO_, __VA_ARGS__))
#define __CC_XPROTO_(type, name) type name

#define __CC_XCALL(...) (__CC_2FOR (__CC_XCALL_, __VA_ARGS__))
#define __CC_XCALL_(type, name) name

#if !defined __cplusplus || !__GNUC_PREREQ (4, 4)

# define __CONST_COV_PROTO(name, attrs, rtype, ...)			\
   extern rtype name __CC_XPROTO(const rtype, __VA_ARGS__) __THROW attrs

# define __CONST_COV_BUILTIN(...) __CONST_COV_PROTO (__VA_ARGS__)

#else

# define __CONST_COV_PROTO_BODY(name, attrs, rtype, ...)		\
  extern rtype name __CC_XPROTO(rtype, __VA_ARGS__)			\
    __THROW __asm (#name) attrs;					\
  extern const rtype name __CC_XPROTO(const rtype, __VA_ARGS__)		\
    __THROW __asm (#name) attrs;

# define __CONST_COV_PROTO(...)						\
   extern "C++"								\
   {									\
     __CONST_COV_PROTO_BODY(__VA_ARGS__)				\
   }									\
   struct __require_semicolon

# ifndef __OPTIMIZE__
#  define __CONST_COV_BUILTIN(...) __CONST_COV_PROTO (__VA_ARGS__)

# else

#  define __CONST_COV_BUILTIN(name, attrs, rtype, ...)			\
  extern "C++"								\
  {									\
    __CONST_COV_PROTO_BODY (name, attrs, rtype, __VA_ARGS__)		\
    __extern_always_inline rtype					\
    name __CC_XPROTO (rtype, __VA_ARGS__) __THROW			\
    { return __builtin_##name __CC_XCALL (rtype, __VA_ARGS__); }	\
    __extern_always_inline const rtype					\
    name __CC_XPROTO (const rtype, __VA_ARGS__) __THROW			\
    { return __builtin_##name __CC_XCALL (rtype, __VA_ARGS__); }	\
  }									\
  struct __require_semicolon

# endif /* __OPTIMIZE__ */
#endif /* C++ and GCC >=4.4 */

#endif /* const-covariance.h */
