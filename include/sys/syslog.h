#ifndef _LIBC_SYS_SYSLOG_H
#define _LIBC_SYS_SYSLOG_H 1
#include <misc/sys/syslog.h>
#ifndef _ISOMAC

/*  Some libc_hidden_ldbl_proto's do not map to a unique symbol when
    redirecting ldouble to _Float128 variants.  We can therefore safely
    directly alias them to their internal name.  */
# if __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI == 1 && IS_IN (libc) && defined SHARED
#  define __syslog_hidden_ldbl_proto(name, alias) \
  extern __typeof (name) __##name __asm__ (__hidden_asmname (#alias));
#  define syslog_hidden_ldbl_proto(name) \
  extern typeof (name) __##name##ieee128; \
  libc_hidden_proto (__##name##ieee128); \
  __syslog_hidden_ldbl_proto (name, __GI___##name##ieee128)
# else
#  define syslog_hidden_ldbl_proto(name) \
  extern __typeof (name) __##name; \
  libc_hidden_proto (__##name)
# endif

syslog_hidden_ldbl_proto (syslog);

/* __vsyslog_internal uses the same mode_flags bits as
   __v*printf_internal; see libio/libioP.h.  */
extern void __vsyslog_internal (int pri, const char *fmt, __gnuc_va_list ap,
				unsigned int mode_flags)
     attribute_hidden
     __attribute__ ((__format__ (__printf__, 2, 0)));

#endif /* _ISOMAC */
#endif /* syslog.h */
