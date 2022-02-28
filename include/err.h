#ifndef _ERR_H
#include <misc/err.h>

/* Prototypes for internal err.h functions.  */
void
__vwarnx_internal (const char *format, __gnuc_va_list ap,
		   unsigned int mode_flags);

void
__vwarn_internal (const char *format, __gnuc_va_list ap,
		   unsigned int mode_flags);

# ifndef _ISOMAC

/*  Some libc_hidden_ldbl_proto's do not map to a unique symbol when
    redirecting ldouble to _Float128 variants.  We can therefore safely
    directly alias them to their internal name.  */
# if __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI == 1 && IS_IN (libc) && defined SHARED
#  define __err_hidden_ldbl_proto(name, alias) \
  extern __typeof (name) __##name __asm__ (__hidden_asmname (#alias));
#  define err_hidden_ldbl_proto(name) \
  extern typeof (name) __##name##ieee128; \
  libc_hidden_proto (__##name##ieee128); \
  __err_hidden_ldbl_proto (name, __GI___##name##ieee128)
# else
#  define err_hidden_ldbl_proto(name) \
  extern __typeof (name) __##name __attribute_copy__ (name); \
  libc_hidden_proto (__##name)
# endif

err_hidden_ldbl_proto (warn);
err_hidden_ldbl_proto (warnx);
err_hidden_ldbl_proto (vwarn);
err_hidden_ldbl_proto (vwarnx);
err_hidden_ldbl_proto (verr);
err_hidden_ldbl_proto (verrx);

# endif /* !_ISOMAC */
#endif /* err.h */
