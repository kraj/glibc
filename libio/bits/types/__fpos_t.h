#ifndef __fpos_t_defined
#define __fpos_t_defined 1

#include <bits/types.h>
#include <bits/types/__mbstate_t.h>

typedef struct
{
  __off_t __pos;
  __mbstate_t __state;
} __fpos_t;

typedef struct
{
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;

#endif
