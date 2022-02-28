/* Wrappers for err.h functions.  IEEE128 version.
   Copyright (C) 2019-2022 Free Software Foundation, Inc.
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

#include <err.h>
#include <stdarg.h>
#include <libio/libioP.h>

#define IEEE128_NAME(name) __ ## name ## ieee128

#define VA(name, ...)							\
{									\
  va_list ap;								\
  va_start (ap, format);						\
  IEEE128_NAME (name) (__VA_ARGS__);					\
  va_end (ap);								\
}

#define IEEE128_ALIAS(name) \
  libc_hidden_def (IEEE128_NAME(name))


void
IEEE128_NAME (vwarn) (const char *format, __gnuc_va_list ap)
{
  __vwarn_internal (format, ap, PRINTF_LDBL_USES_FLOAT128);
}
IEEE128_ALIAS (vwarn)

void
IEEE128_NAME (vwarnx) (const char *format, __gnuc_va_list ap)
{
  __vwarnx_internal (format, ap, PRINTF_LDBL_USES_FLOAT128);
}
IEEE128_ALIAS (vwarnx)

void
IEEE128_NAME (warn) (const char *format, ...)
{
  VA (vwarn, format, ap)
}
IEEE128_ALIAS (warn)

void
IEEE128_NAME (warnx) (const char *format, ...)
{
  VA (vwarnx, format, ap)
}
IEEE128_ALIAS (warnx)

void
IEEE128_NAME (verr) (int status, const char *format, __gnuc_va_list ap)
{
  IEEE128_NAME (vwarn) (format, ap);
  exit (status);
}
IEEE128_ALIAS (verr)

void
IEEE128_NAME (verrx) (int status, const char *format, __gnuc_va_list ap)
{
  IEEE128_NAME (vwarnx) (format, ap);
  exit (status);
}
IEEE128_ALIAS (verrx)

void
IEEE128_NAME (err) (int status, const char *format, ...)
{
  VA (verr, status, format, ap)
}

void
IEEE128_NAME (errx) (int status, const char *format, ...)
{
  VA (verrx, status, format, ap)
}
