/* Return backtrace of current program state.
   Copyright (C) 2000-2020 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky <schwidefsky@de.ibm.com>.
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

#include <execinfo.h>
#include <stddef.h>
#include <stdlib.h>
#include <unwind-link.h>

/* This is a global variable set at program start time.  It marks the
   highest used stack address.  */
extern void *__libc_stack_end;


/* This is the stack layout we see for every non-leaf function.
           size                    offset
    %r15 ->    +------------------+
             4 | back chain       |  0
             4 | end of stack     |  4
             8 | glue             |  8
             8 | scratch          | 16
            40 | save area r6-r15 | 24
            16 | save area f4,f6  | 64
            16 | empty            | 80
               +------------------+
   r14 in the save area holds the return address.
*/

struct layout
{
  int back_chain;
  int end_of_stack;
  int glue[2];
  int scratch[2];
  int save_grps[10];
  int save_fp[4];
  int empty[2];
};

struct trace_arg
{
  void **array;
  struct unwind_link *unwind_link;
  int cnt, size;
};

#ifdef SHARED
static int
__backchain_backtrace (void **array, int size)
{
  /* We assume that all the code is generated with frame pointers set.  */
  struct layout *stack;
  int cnt = 0;

  __asm__ ("LR  %0,%%r15" : "=d" (stack) );
  /* We skip the call to this function, it makes no sense to record it.  */
  stack = (struct layout *) stack->back_chain;
  while (cnt < size)
    {
      if (stack == NULL || (void *) stack > __libc_stack_end)
	/* This means the address is out of range.  Note that for the
	   toplevel we see a frame pointer with value NULL which clearly is
	   out of range.  */
	break;

      array[cnt++] = (void *) (stack->save_grps[8] & 0x7fffffff);

      stack = (struct layout *) stack->back_chain;
    }

  return cnt;
}
#endif

static _Unwind_Reason_Code
backtrace_helper (struct _Unwind_Context *ctx, void *a)
{
  struct trace_arg *arg = a;

  /* We are first called with address in the __backtrace function.
     Skip it.  */
  if (arg->cnt != -1)
    arg->array[arg->cnt]
      = (void *) UNWIND_LINK_PTR (arg->unwind_link, _Unwind_GetIP) (ctx);
  if (++arg->cnt == arg->size)
    return _URC_END_OF_STACK;
  return _URC_NO_REASON;
}

int
__backtrace (void **array, int size)
{
  struct trace_arg arg =
    {
     .array = array,
     .unwind_link = __libc_unwind_link_get (),
     .size = size,
     .cnt = -1
    };

  if (size <= 0)
    return 0;

#ifdef SHARED
  if (arg.unwind_link == NULL)
    return __backchain_backtrace (array, size);
#endif

  UNWIND_LINK_PTR (arg.unwind_link, _Unwind_Backtrace)
    (backtrace_helper, &arg);

  return arg.cnt != -1 ? arg.cnt : 0;
}

weak_alias (__backtrace, backtrace)
libc_hidden_def (__backtrace)
