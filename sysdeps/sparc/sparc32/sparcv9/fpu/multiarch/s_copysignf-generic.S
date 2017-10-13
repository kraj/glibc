#define __copysignf __copysignf_generic
#undef weak_alias
#define weak_alias(a, b)
#include <sysdeps/sparc/sparc32/fpu/s_copysignf.S>
