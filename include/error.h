#ifndef _ERROR_H
#include <misc/error.h>

# ifndef _ISOMAC

#include <stdarg.h>

void
__error_internal (int status, int errnum, const char *message,
		  va_list args, unsigned int mode_flags) __COLD;

void
__error_at_line_internal (int status, int errnum, const char *file_name,
			  unsigned int line_number, const char *message,
			  va_list args, unsigned int mode_flags) __COLD;

# endif /* !_ISOMAC */
#endif
