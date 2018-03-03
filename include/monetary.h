#include <stdlib/monetary.h>
#ifndef _ISOMAC
#include <stdarg.h>

extern ssize_t __vstrfmon_l_internal (char *s, size_t maxsize, locale_t loc,
                                      const char *format, va_list ap,
                                      unsigned int flags);

/* Flags for __vstrfmon_l_internal.  */
#define STRFMON_LDBL_IS_DBL 0x0001

#endif
