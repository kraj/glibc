/* Pthread macros.  Linux/x86 version.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

#include_next <pthreaddef.h>

/* Need saved_mask in cancel_jmp_buf.  */
#define NEED_SAVED_MASK_IN_CANCEL_JMP_BUF 1

/* Wee need to copy feature_1 in pthread_create.  */
#define THREAD_COPY_ADDITONAL_INFO(descr)				\
  ((descr)->header.feature_1						\
   = THREAD_GETMEM (THREAD_SELF, header.feature_1))

/* Use the compatible struct __cancel_jmp_buf_tag if shadow stack is
   disabled.  */
#undef UNWIND_BUF_PRIV
#define UNWIND_BUF_PRIV(self,p) \
  (__extension__ ({							\
     unsigned int feature_1 = THREAD_GETMEM (self, header.feature_1);	\
     (((feature_1 & (1 << 1)) == 0)					\
      ? &((p)->compat.priv) : &((p)->full.priv));}))
