/* 4.4BSD utility functions for error messages.
   Copyright (C) 1995-2022 Free Software Foundation, Inc.
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

#ifndef	_ERR_H
#define	_ERR_H	1

#include <features.h>

#define	__need___va_list
#include <stdarg.h>
#ifndef	__GNUC_VA_LIST
# define __gnuc_va_list	void *
#endif

__BEGIN_DECLS

/* Print "program: ", FORMAT, ": ", the standard error string for errno,
   and a newline, on stderr.  */
extern void __REDIRECT_LDBL (warn, (const char *__format, ...),
			     __warnieee128, __nldbl_warn)
     __attribute__ ((__format__ (__printf__, 1, 2)));
extern void __REDIRECT_LDBL (vwarn, (const char *__format, __gnuc_va_list),
			     __vwarnieee128, __nldbl_vwarn)
     __attribute__ ((__format__ (__printf__, 1, 0)));

/* Likewise, but without ": " and the standard error string.  */
extern void __REDIRECT_LDBL (warnx, (const char *__format, ...),
			     __warnxieee128, __nldbl_warnx)
     __attribute__ ((__format__ (__printf__, 1, 2)));
extern void __REDIRECT_LDBL (vwarnx, (const char *__format, __gnuc_va_list),
			     __vwarnxieee128, __nldbl_vwarnx)
     __attribute__ ((__format__ (__printf__, 1, 0)));

/* Likewise, and then exit with STATUS.  */
extern void __REDIRECT_LDBL (err, (int __status, const char *__format, ...),
			     __errieee128, __nldbl_err)
     __attribute__ ((__noreturn__, __format__ (__printf__, 2, 3)));
extern void __REDIRECT_LDBL (verr, (int __status, const char *__format,
				    __gnuc_va_list),
			     __verrieee128, __nldbl_verr)
     __attribute__ ((__noreturn__, __format__ (__printf__, 2, 0)));
extern void __REDIRECT_LDBL (errx, (int __status, const char *__format, ...),
			     __errxieee128, __nldbl_errx)
     __attribute__ ((__noreturn__, __format__ (__printf__, 2, 3)));
extern void __REDIRECT_LDBL (verrx, (int __status, const char *__format,
				     __gnuc_va_list),
			     __verrxieee128, __nldbl_verrx)
     __attribute__ ((__noreturn__, __format__ (__printf__, 2, 0)));

__END_DECLS

#endif	/* err.h */
