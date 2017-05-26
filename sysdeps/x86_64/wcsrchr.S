#define USE_AS_WCSRCHR 1
#define strrchr wcsrchr

#include "strrchr.S"

strong_alias (wcsrchr, __wcsrchr)
