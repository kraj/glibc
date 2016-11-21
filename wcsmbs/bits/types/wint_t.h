#ifndef _BITS_TYPES_WINT_T_H
#define _BITS_TYPES_WINT_T_H 1

/* Integral type unchanged by default argument promotions that can
   hold any value corresponding to members of the extended character
   set, as well as at least one value that does not correspond to any
   member of the extended character set.  */
#ifndef __WINT_TYPE__
# define __WINT_TYPE__ unsigned int
#endif

/* GCC's stddef.h may or may not define wint_t.  If it does, it defines
   _WINT_T to indicate that it has.  */
#ifndef _WINT_T
# define _WINT_T 1
typedef __WINT_TYPE__ wint_t;
#endif

/* GCC's stddef.h may or may not know to put wint_t in namespace std in C++.
   Fortunately, redundant using-declarations are harmless.  */
#if defined __cplusplus && defined _GLIBCPP_USE_NAMESPACES
__BEGIN_NAMESPACE_STD
using ::wint_t;
__END_NAMESPACE_STD
#endif

#endif
