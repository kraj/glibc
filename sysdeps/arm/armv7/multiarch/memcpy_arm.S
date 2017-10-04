#define memcpy __memcpy_arm
#include "memcpy_impl.S"

strong_alias (__memcpy_arm, __aeabi_memcpy)
strong_alias (__memcpy_arm, __aeabi_memcpy4)
strong_alias (__memcpy_arm, __aeabi_memcpy8)
