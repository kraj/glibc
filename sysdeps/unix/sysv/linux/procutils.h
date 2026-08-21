/* Utilities functions to read/parse Linux procfs and sysfs.
   Copyright (C) 2023-2026 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
   <https://www.gnu.org/licenses/>.  */

#ifndef _PROCUTILS_H
#define _PROCUTILS_H

#include <stdbool.h>
#include <stddef.h>

typedef int (*procutils_closure_t) (const char *line, void *arg);

enum procutils_read_result_t
{
  procutils_read_stop,
  procutils_read_eof,
  procutils_read_error
};

/* Open and read the path FILENAME, line per line, and call CLOSURE with
   argument ARG on each line.  The read is done with non-cancellable
   calls using BUFFER of BUFFER_SIZE bytes as scratch area, and the line
   is null terminated (the '\n' is not included).

   A line longer than BUFFER_SIZE - 1 characters is passed to CLOSURE
   truncated to BUFFER_SIZE - 1 characters, and the rest of the line up
   to the next '\n' is discarded.

   The CLOSURE should return 0 if the read should continue, otherwise
   the function stops reading and returns early.

   It returns procutils_read_stop if CLOSURE returned a value different
   than 0, procutils_read_eof if the file was fully read, or
   procutils_read_error if the file could not be opened or a read error
   occurred (with errno set).  */
enum procutils_read_result_t
__libc_procutils_read_file (const char *filename,
			    char *buffer, size_t buffer_size,
			    procutils_closure_t closure,
			    void *arg) attribute_hidden;

#endif
