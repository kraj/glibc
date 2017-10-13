#include <math_ldbl_opt.h>

#define __copysign __copysign_generic
#undef weak_alias
#define weak_alias(a, b)
#undef compat_symbol
#define compat_symbol(a, b, c, d)
#include <sysdeps/sparc/sparc32/fpu/s_copysign.S>
