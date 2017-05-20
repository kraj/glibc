#define AS_WCSLEN
#define strlen	__wcslen

#include "strlen.S"

weak_alias(__wcslen, wcslen)
