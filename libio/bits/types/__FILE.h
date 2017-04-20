#ifndef ____FILE_defined
#define ____FILE_defined 1

/* Note: the struct tag is _IO_FILE rather than __FILE for historical
   reasons.  It potentially appears in C++ mangled names and therefore
   cannot be changed.  This file must be kept in sync with FILE.h and
   FILE_internals.h.  */

struct _IO_FILE;
typedef struct _IO_FILE __FILE;

#endif
