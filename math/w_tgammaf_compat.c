/* w_gammaf.c -- float version of w_gamma.c.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <math_private.h>
#include <math-svid-compat.h>
#include <libm-alias-float.h>

#if LIBM_SVID_COMPAT
float
__tgammaf(float x)
{
	float y = __ieee754_gammaf_r(x, NULL);

	if(__glibc_unlikely (!isfinite (y) || y == 0)
	   && (isfinite (x) || (isinf (x) && x < 0.0))
	   && _LIB_VERSION != _IEEE_) {
	  if (x == (float)0.0)
	    /* tgammaf pole */
	    return __kernel_standard_f(x, x, 150);
	  else if(floorf(x)==x&&x<0.0f)
	    /* tgammaf domain */
	    return __kernel_standard_f(x, x, 141);
	  else if (y == 0)
	    /* tgammaf underflow */
	    __set_errno (ERANGE);
	  else
	    /* tgammaf overflow */
	    return __kernel_standard_f(x, x, 140);
	}
	return y;
}
libm_alias_float (__tgamma, tgamma)
#endif
