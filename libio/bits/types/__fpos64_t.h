#ifndef _____fpos64_t_defined
#define _____fpos64_t_defined 1

#include <bits/types.h>
#include <bits/types/__mbstate_t.h>

typedef struct
{
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;

#endif
