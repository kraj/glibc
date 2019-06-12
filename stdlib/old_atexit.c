#include <exit.h>
#include <dso_handle.h>
#include <shlib-compat.h>

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_2_2)
int attribute_compat_text_section
__dyn_atexit (void (*func) (void))
{
  return __cxa_atexit ((void (*) (void *)) func, NULL, __dso_handle);
}
compat_symbol (libc, __dyn_atexit, atexit, GLIBC_2_0);
#endif
