#include "nldbl-compat.h"

int
attribute_hidden
scanf (const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vfscanf (stdin, fmt, arg);
  va_end (arg);

  return done;
}
