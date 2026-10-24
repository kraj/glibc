#if IS_IN (libc)

# include "x86-avx-rtm-vecs.h"

#ifndef MEMMOVE_SYMBOL
# define MEMMOVE_SYMBOL(p,s)	p##_avx_##s##_rtm
#endif

# include "memmove-vec-unaligned-erms.S"
#endif
