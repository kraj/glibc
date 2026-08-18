#ifndef _SYS_FUTEX_H
#include_next <sys/futex.h>

# ifndef _ISOMAC

# include <futex-internal.h>
# include <struct___timespec64.h>

static __always_inline _Bool
__futex_check_flags (unsigned int flags)
{
  return (flags & ~(unsigned int) FUTEX_PRIVATE_FLAG) == 0;
}

# if __TIMESIZE == 64
#  define __futex_timedwait_time64 futex_timedwait
# else
extern int __futex_timedwait_time64 (uint32_t *futexp, uint32_t expected,
				     clockid_t clockid,
				     const struct __timespec64 *abstime,
				     unsigned int flags)
     __nonnull ((1));
libc_hidden_proto (__futex_timedwait_time64)
# endif

# endif /* !_ISOMAC */
#endif
