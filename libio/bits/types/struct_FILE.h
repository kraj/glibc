#ifndef __struct_FILE_defined
#define __struct_FILE_defined 1

/* Caution: The contents of this file are not part of the official
   stdio.h API.  However, much of it is part of the official *binary*
   interface, and therefore cannot be changed.  */

#if defined _IO_USE_OLD_IO_FILE && !defined _LIBC
# error "_IO_USE_OLD_IO_FILE should only be defined when building libc itself"
#endif

#if defined _IO_lock_t_defined && !defined _LIBC
# error "_IO_lock_t_defined should only be defined when building libc itself"
#endif

#include <bits/types.h>

struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;

/* During the build of glibc itself, _IO_lock_t will already have been
   defined by internal headers.  */
#ifndef _IO_lock_t_defined
typedef void _IO_lock_t;
#endif

struct _IO_FILE
{
  int _flags;		/* High-order word is _IO_MAGIC; rest is flags. */

  /* The following pointers correspond to the C++ streambuf protocol. */
  char *_IO_read_ptr;	/* Current read pointer */
  char *_IO_read_end;	/* End of get area. */
  char *_IO_read_base;	/* Start of putback+get area. */
  char *_IO_write_base;	/* Start of put area. */
  char *_IO_write_ptr;	/* Current put pointer. */
  char *_IO_write_end;	/* End of put area. */
  char *_IO_buf_base;	/* Start of reserve area. */
  char *_IO_buf_end;	/* End of reserve area. */

  /* The following fields are used to support backing up and undo. */
  char *_IO_save_base; /* Pointer to start of non-current get area. */
  char *_IO_backup_base;  /* Pointer to first valid character of backup area */
  char *_IO_save_end; /* Pointer to end of non-current get area. */

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;
  int _flags2;
  __off_t _old_offset; /* This used to be _offset but it's too small.  */

  /* 1+column number of pbase(); 0 is unknown. */
  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];

  _IO_lock_t *_lock;
#ifdef _IO_USE_OLD_IO_FILE
};

struct _IO_FILE_complete
{
  struct _IO_FILE _file;
#endif
  __off64_t _offset;
  /* Wide character stream stuff.  */
  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  size_t __pad5;
  int _mode;
  /* Make sure we don't get into trouble again.  */
  char _unused2[15 * sizeof (int) - 4 * sizeof (void *) - sizeof (size_t)];
};

/* These macros are used by bits/stdio.h and libio.h.  */
#define __getc_unlocked_body(_fp)					\
  (__glibc_unlikely ((_fp)->_IO_read_ptr >= (_fp)->_IO_read_end)	\
   ? __uflow (_fp) : *(unsigned char *) (_fp)->_IO_read_ptr++)

#define __putc_unlocked_body(_ch, _fp)					\
  (__glibc_unlikely ((_fp)->_IO_write_ptr >= (_fp)->_IO_write_end)	\
   ? __overflow (_fp, (unsigned char) (_ch))				\
   : (unsigned char) (*(_fp)->_IO_write_ptr++ = (_ch)))

#define _IO_EOF_SEEN 0x10
#define __feof_unlocked_body(_fp) (((_fp)->_flags & _IO_EOF_SEEN) != 0)

#define _IO_ERR_SEEN 0x20
#define __ferror_unlocked_body(_fp) (((_fp)->_flags & _IO_ERR_SEEN) != 0)

#endif
