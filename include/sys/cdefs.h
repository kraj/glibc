#ifndef _SYS_CDEFS_H

/* This is outside of _ISOMAC to enforce that _Static_assert always
   uses the two-argument form.  This can be removed once the minimum
   GCC version used to compile glibc is GCC 9.1.  */
#ifndef __cplusplus
# define _Static_assert(expr, diagnostic) _Static_assert (expr, diagnostic)
#endif

#include <misc/sys/cdefs.h>

#ifndef _ISOMAC
/* The compiler will optimize based on the knowledge the parameter is
   not NULL.  This will omit tests.  A robust implementation cannot allow
   this so when compiling glibc itself we ignore this attribute.  */
# undef __nonnull
# define __nonnull(params)

extern void __chk_fail (void) __attribute__ ((__noreturn__));
libc_hidden_proto (__chk_fail)
rtld_hidden_proto (__chk_fail)

#endif /* !defined _ISOMAC */

#endif
