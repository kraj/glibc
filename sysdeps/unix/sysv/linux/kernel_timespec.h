/* Definition of `struct timespec' used in the kernel, 32- and 64-bit variants.  */

#include <bits/types.h>

struct kernel_timespec32
  {
    __time_t tv_sec;
    __syscall_slong_t tv_nsec;
  };

struct kernel_timespec64
  {
    __time64_t tv_sec;
    __syscall_squad_t tv_nsec;
  };
