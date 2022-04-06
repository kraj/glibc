#include <stdio.h>
#include <errno.h>
#include <libintl.h>
#include <array_length.h>

#ifndef ERR_MAP
# define ERR_MAP(n) n
#endif

const char *const _sys_errlist_internal[] =
  {
#define _S(n, str)         [ERR_MAP(n)] = str,
#include <errlist.h>
#undef _S
  };
const size_t _sys_errlist_internal_len = array_length (_sys_errlist_internal);

/* Include to get the definitions for sys_nerr/_sys_nerr.  */
#include <errlist-compat-data.h>
