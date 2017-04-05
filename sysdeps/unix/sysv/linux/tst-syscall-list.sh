#!/bin/bash
# Consistency checks for the system call list
# Copyright (C) 2017 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
#
# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

export LC_ALL=C
set -e
set -o pipefail

if test $# != 2 ; then
    echo "error: wrong number of arguments: $#"
    exit 1
fi

list_nr="$1"
list_sys="$2"

errors=0

# Use getpid as a system call which is expected to be always defined.
# alpha uses getxpid instead, so it is permitted as an alternative.
if ! grep -E -q '^getx?pid$' -- "$list_nr" ; then
    echo "error: __NR_getpid not defined"
    errors=1
fi
if ! grep -E -q '^getx?pid$' -- "$list_sys" ; then
    echo "error: SYS_getpid not defined"
    errors=1
fi

comm_1="$(mktemp)"
comm_2="$(mktemp)"
comm_result="$(mktemp)"
cleanup () {
    rm -f -- "$comm_1" "$comm_2" "$comm_result"
}
trap cleanup 0

sort -o "$comm_1" -- "$list_nr"
sort -o "$comm_2" -- "$list_sys"

# Check for missing SYS_* macros.
comm --check-order -2 -3 -- "$comm_1" "$comm_2" > "$comm_result"
if test -s "$comm_result"; then
    echo "error: These system calls need to be added to syscall-names.list:"
    cat -- "$comm_result"
    errors=1
fi

# Check for additional SYS_* macros.
comm --check-order -1 -3 -- "$comm_1" "$comm_2" > "$comm_result"
if test -s "$comm_result"; then
    echo "error: The following system calls have unexpected SYS_* macros:"
    cat -- "$comm_result"
    errors=1
fi

exit "$errors"
