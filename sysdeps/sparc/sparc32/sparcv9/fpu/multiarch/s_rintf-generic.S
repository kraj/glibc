#define __rintf __rintf_generic
#undef weak_alias
#define weak_alias(a, b)
#include <sysdeps/sparc/sparc32/sparcv9/fpu/s_rintf.S>
