#if __has_include_next (<stdckdint.h>)
# include_next <stdckdint.h>
#else
# define ckd_add(r, a, b) ((_Bool) __builtin_add_overflow (a, b, r))
# define ckd_sub(r, a, b) ((_Bool) __builtin_sub_overflow (a, b, r))
# define ckd_mul(r, a, b) ((_Bool) __builtin_mul_overflow (a, b, r))
#endif
