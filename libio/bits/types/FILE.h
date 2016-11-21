#ifndef __FILE_defined
#define __FILE_defined 1

struct _IO_FILE;

__BEGIN_NAMESPACE_STD
/* The opaque type of streams.  This is the definition used elsewhere.  */
typedef struct _IO_FILE FILE;
__END_NAMESPACE_STD
#if defined __USE_LARGEFILE64 || defined __USE_POSIX \
    || defined __USE_ISOC99 || defined __USE_XOPEN \
    || defined __USE_POSIX2
__USING_NAMESPACE_STD(FILE)
#endif

#endif
