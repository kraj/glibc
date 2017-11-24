/* Internal prototrypes for multibyte and wide character functions.
   i386 version.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

extern __typeof (wcpncpy) __wcpncpy attribute_hidden;
extern __typeof (wcscat) __wcscat attribute_hidden __attribute_pure__;
extern __typeof (wcschrnul) __wcschrnul attribute_hidden __attribute_pure__;
extern __typeof (wcsncpy) __wcsncpy attribute_hidden;
extern __typeof (wcsnlen) __wcsnlen attribute_hidden __attribute_pure__;
