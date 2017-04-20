/* Internal structure of a FILE object.
   Copyright (C) 1991-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Per Bothner <bothner@cygnus.com>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */


#ifndef _FILE_internals_defined
#define _FILE_internals_defined 1

/* This file exposes just enough of the internal structure of a FILE
   object to permit the optimizations in bits/stdio.h.

   Note: the _IO_ prefixes on struct tags and field names are for
   historical reasons.  The GNU C Library no longer supports the
   "libio" extension to stdio.

   This file must be kept in sync with __FILE.h, FILE.h, and internal
   headers.  */

#include <bits/types.h>

/* During the build of glibc itself, _IO_lock_t will already have been
   defined by internal headers.  */
#ifndef _IO_lock_t_defined
typedef void _IO_lock_t;
#define _IO_lock_t_defined 1
#endif

struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;

struct _IO_FILE
{
  int _flags;		/* High-order word is _IO_MAGIC; rest is flags. */
  /* The following pointers correspond to the C++ streambuf protocol. */
  char* _IO_read_ptr;	/* Current read pointer */
  char* _IO_read_end;	/* End of get area. */
  char* _IO_read_base;	/* Start of putback+get area. */
  char* _IO_write_base;	/* Start of put area. */
  char* _IO_write_ptr;	/* Current put pointer. */
  char* _IO_write_end;	/* End of put area. */
  char* _IO_buf_base;	/* Start of reserve area. */
  char* _IO_buf_end;	/* End of reserve area. */
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

  /* Fields below this point are not present in the "old" FILE structure.  */
  __off64_t _offset;
  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  size_t __pad5;
  int _mode;

  /* Make sure we don't get into trouble again.  */
  char _unused2[15 * sizeof (int) - 4 * sizeof (void *) - sizeof (size_t)];
};

/* Many more flags are defined internally.  */
#ifndef _IO_EOF_SEEN
# define _IO_EOF_SEEN 0x10
#elif _IO_EOF_SEEN != 0x10
# error "FILE_internals.h out of sync with libio.h (_IO_EOF_SEEN)"
#endif

#ifndef _IO_ERR_SEEN
# define _IO_ERR_SEEN 0x20
#elif _IO_ERR_SEEN != 0x20
# error "FILE_internals.h out of sync with libio.h (_IO_ERR_SEEN)"
#endif

#ifndef _IO_USER_LOCK
# define _IO_USER_LOCK 0x8000
#elif _IO_USER_LOCK != 0x8000
# error "FILE_internals.h out of sync with libio.h (_IO_USER_LOCK)"
#endif

#endif
