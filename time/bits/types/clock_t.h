#ifndef _BITS_TYPES_CLOCK_T_H
#define _BITS_TYPES_CLOCK_T_H 1

#include <bits/types.h>

__BEGIN_NAMESPACE_STD
/* Returned by `clock'.  */
typedef __clock_t clock_t;
__END_NAMESPACE_STD

#if defined __USE_XOPEN || defined __USE_POSIX
__USING_NAMESPACE_STD(clock_t)
#endif

#endif
