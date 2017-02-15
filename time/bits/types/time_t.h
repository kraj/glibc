#ifndef __time_t_defined
#define __time_t_defined 1

#include <bits/types.h>

__BEGIN_NAMESPACE_STD
/* Returned by `time'.  */
#ifdef __USE_TIME_BITS64
typedef __time64_t time_t;
#else
typedef __time_t time_t;
__END_NAMESPACE_STD
# ifdef __USE_POSIX
__USING_NAMESPACE_STD(time_t)
# endif
#endif

#endif
