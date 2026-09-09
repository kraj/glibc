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

#include <libc-lock.h>
#include <not-cancel.h>
#include <procutils.h>
#include <stdbool.h>
#include <string.h>

struct line_reader
{
  int fd;
  char *buffer;
  /* The last byte of the buffer is reserved for the null terminator.  */
  char *buffer_end;
  char *cp;   /* Start of the unprocessed data.  */
  char *re;   /* End of the unprocessed data.  */
  bool eof;   /* No more data to read.  */
  bool skip;  /* Discard the input up to the next '\n', the initial part of
		 the line has already been returned truncated.  */
};

enum next_line_result_t
{
  next_line_ok,
  next_line_eof,
  next_line_error
};

/* Read the next line into *R.  Return next_line_ok on success,
   next_line_eof on EOF, or next_line_error on read error.  */
static enum next_line_result_t
next_line (struct line_reader *lr, char **r)
{
  while (true)
    {
      char *nl = memchr (lr->cp, '\n', lr->re - lr->cp);
      if (nl != NULL)
	{
	  char *line = lr->cp;
	  *nl = '\0';
	  lr->cp = nl + 1;
	  if (lr->skip)
	    {
	      /* End of a line longer than the buffer, start over.  */
	      lr->skip = false;
	      continue;
	    }
	  *r = line;
	  return next_line_ok;
	}

      if (lr->eof)
	{
	  if (lr->cp == lr->re || lr->skip)
	    return next_line_eof;
	  /* Last line without a trailing newline.  */
	  *lr->re = '\0';
	  *r = lr->cp;
	  lr->cp = lr->re;
	  return next_line_ok;
	}

      /* Move the partial line to the start of the buffer to maximize
	 the read size.  */
      if (lr->cp != lr->buffer)
	{
	  memmove (lr->buffer, lr->cp, lr->re - lr->cp);
	  lr->re = lr->buffer + (lr->re - lr->cp);
	  lr->cp = lr->buffer;
	}

      if (lr->re == lr->buffer_end)
	{
	  /* A line longer than the buffer.  Consume the buffered data
	     and, for the initial part of the line, also return it
	     truncated.  */
	  lr->cp = lr->re = lr->buffer;
	  if (!lr->skip)
	    {
	      *lr->buffer_end = '\0';
	      *r = lr->buffer;
	      lr->skip = true;
	      return next_line_ok;
	    }
	}

      ssize_t n = TEMP_FAILURE_RETRY (
	__read_nocancel (lr->fd, lr->re, lr->buffer_end - lr->re));
      if (n < 0)
	return next_line_error;
      if (n == 0)
	lr->eof = true;
      lr->re += n;
    }
}

static void
close_handler (void *arg)
{
  __close_nocancel_nostatus (*(int *) arg);
}

enum procutils_read_result_t
__libc_procutils_read_file (const char *filename, char *buffer,
			    size_t buffer_size, procutils_closure_t closure,
			    void *arg)
{
  struct line_reader lr =
    {
      .buffer = buffer,
      .buffer_end = buffer + buffer_size - 1,
      .cp = buffer,
      .re = buffer,
    };

  lr.fd = TEMP_FAILURE_RETRY (
    __open64_nocancel (filename, O_RDONLY | O_CLOEXEC));
  if (lr.fd == -1)
    return procutils_read_error;

  /* The cleanup handler is required to avoid leaking the descriptor if
     the thread is asynchronously canceled.  */
  enum next_line_result_t r;
  __libc_cleanup_push (close_handler, &lr.fd);

  char *line;
  while ((r = next_line (&lr, &line)) == next_line_ok)
    if (closure (line, arg) != 0)
      break;

  __close_nocancel_nostatus (lr.fd);

  __libc_cleanup_pop (0);

  if (r == next_line_error)
    return procutils_read_error;
  return r == next_line_ok ? procutils_read_stop : procutils_read_eof;
}
