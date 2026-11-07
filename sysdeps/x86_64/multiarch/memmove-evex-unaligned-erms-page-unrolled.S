#ifndef MEMMOVE_SYMBOL
# define MEMMOVE_SYMBOL(p,s)	p##_evex_##s##_page_unrolled
#endif
#define MEMMOVE_VEC_LARGE_IMPL	"memmove-vec-large-page-unrolled.S"
#define PAGE_UNROLLED	1
#include "memmove-evex-unaligned-erms.S"
