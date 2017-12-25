#ifndef __cookie_io_functions_t_defined
#define __cookie_io_functions_t_defined 1

#include <bits/types.h>

/* Functions to do I/O and file management for a stream.  */

/* Read NBYTES bytes from COOKIE into a buffer pointed to by BUF.
   Return number of bytes read.  */
typedef __ssize_t cookie_read_function_t (void *__cookie, char *__buf, size_t __nbytes);

/* Write NBYTES bytes pointed to by BUF to COOKIE.  Write all NBYTES bytes
   unless there is an error.  Return number of bytes written.  If
   there is an error, return 0 and do not write anything.  If the file
   has been opened for append (__mode.__append set), then set the file
   pointer to the end of the file and then do the write; if not, just
   write at the current file pointer.  */
typedef __ssize_t cookie_write_function_t (void *__cookie, const char *__buf,
                                           size_t __nbytes);

/* Move COOKIE's file position to *POS bytes from the
   beginning of the file (if W is SEEK_SET),
   the current position (if W is SEEK_CUR),
   or the end of the file (if W is SEEK_END).
   Set *POS to the new file position.
   Returns zero if successful, nonzero if not.  */
typedef int cookie_seek_function_t (void *__cookie, __off64_t *__pos, int __w);

/* Close COOKIE.  */
typedef int cookie_close_function_t (void *__cookie);

/* The structure with the cookie function pointers.  */
typedef struct
{
  cookie_read_function_t *read;		/* Read bytes.  */
  cookie_write_function_t *write;	/* Write bytes.  */
  cookie_seek_function_t *seek;		/* Seek/tell file position.  */
  cookie_close_function_t *close;	/* Close file.  */
} cookie_io_functions_t;

#endif
