#ifndef MEMMOVE_SYMBOL
# define MEMMOVE_SYMBOL(p,s)	p##_avx_##s##_page_unrolled
#endif
#define MEMMOVE_VEC_LARGE_IMPL	"memmove-vec-large-page-unrolled.S"
#include "memmove-avx-unaligned-erms.S"
