#ifndef __error_t_defined
#define __error_t_defined 1

/* The type error_t, a debugging aid.  With sufficiently new compilers
   you can type 'p (error_t) errno' in GDB and see the symbolic name
   of the errno value.  */
#if __GNUC_PREREQ (4, 5)
# include <bits/errno-enum.h>
typedef enum __error_t_codes error_t;
# else
typedef int error_t;
# endif

#endif
