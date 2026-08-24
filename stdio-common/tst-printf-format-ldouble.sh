#!/bin/bash
# Testing of long double printf conversions.
# Copyright (C) 2024-2026 Free Software Foundation, Inc.
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

set -e

xprintf=$1; shift
format=$1; shift
common_objpfx=$1; shift
test_program_prefix=$1; shift

status=0

rc=0
(set -o pipefail
 ${test_program_prefix} \
  ${common_objpfx}stdio-common/tst-printf-format-${xprintf}-ldouble $format |
   ${PYTHON:-python3} tst-printf-format.py 2>&1 |
   head -n 1 |
   sed "s/^/Conversion $format output error, first line:\n/") 2>&1 ||
  rc=$?

# The generator produces no records and reports an unsupported status for
# a conversion the verification cannot model for the long double format in
# use; see UNSUPPORTED_CONVS in tst-printf-format-skeleton-ldouble.c.
case $rc in
0)  echo Verifying $format ;;
77) echo Unsupported $format; status=77 ;;
*)  echo Verifying $format; status=1 ;;
esac

exit $status
