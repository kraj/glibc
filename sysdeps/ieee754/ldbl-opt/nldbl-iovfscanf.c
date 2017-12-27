#include "nldbl-compat.h"

int
attribute_hidden
_IO_vfscanf (FILE *s, const char *fmt, __gnuc_va_list ap, int *errp)
{
  return __nldbl__IO_vfscanf (s, fmt, ap, errp);
}
