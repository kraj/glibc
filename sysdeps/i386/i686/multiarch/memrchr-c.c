#if IS_IN (libc)
# define MEMRCHR  __memrchr_ia32
# undef weak_alias
# define weak_alias(a,b)
# include <string.h>
extern void *__memrchr_ia32 (const void *, int, size_t);
#endif

#include "string/memrchr.c"
