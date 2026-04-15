#!/bin/sh
# This script checks that malloc.c does not use any external API
# functions.
# It is important that whenever an internal function needs to used e.g.
# __libc_foo() it actually uses __libc_foo_core() symbol. If it uses a
# non-_core symbol, the returned result may not be suitable for the
# subsequent use of it internally.
# The non-_core malloc functions return and accept user-pointers which
# are different from the internal-pointers that are used by the _core
# functions.

# Copyright (C) 2026 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <https://www.gnu.org/licenses/>.

# This script accepts list of source files that need to be grep-ed for
# certain symbols and the result should be no matches except for the
# alias declarations

status=0

for src; do
  echo "checking $src..."
  for fun in \
    __libc_malloc \
    __libc_calloc \
    __libc_memalign \
    __libc_valloc \
    __libc_pvalloc \
    __libc_realloc \
    __libc_free; do
    grep -nw "$fun" $src | egrep -vw "^[0-9]+:(strong_alias|weak_alias)" && {
      good=$fun"_core"
      echo "error: code in $src should not use '$fun' (use '$good' instead)"
      status=1
    }
  done
done

exit $status
