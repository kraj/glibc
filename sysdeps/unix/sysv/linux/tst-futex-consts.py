#!/usr/bin/python3
# Test that glibc's sys/futex.h constants match the kernel's.
# Copyright (C) 2026 Free Software Foundation, Inc.
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
# <https://www.gnu.org/licenses/>.

import argparse
import sys

import glibcextract
import glibcsyscalls


def main():
    """The main entry point."""
    parser = argparse.ArgumentParser(
        description="Test that glibc's sys/futex.h constants "
        "match the kernel's.")
    parser.add_argument('--cc', metavar='CC',
                        help='C compiler (including options) to use')
    args = parser.parse_args()

    if glibcextract.compile_c_snippet(
            '#include <linux/futex.h>',
            args.cc).returncode != 0:
        sys.exit (77)

    linux_version_headers = glibcsyscalls.linux_kernel_version(args.cc)
    # Constants in glibc were updated to match Linux v7.1.  When glibc
    # constants are updated this value should be updated to match the
    # released kernel version from which the constants were taken, and
    # the futex_waitv per-waiter flags handling should be revisited for
    # any new FUTEX2_* flag.
    linux_version_glibc = (7, 1)
    def check(cte, exclude=None):
        return glibcextract.compare_macro_consts(
                '#include <sys/futex.h>\n',
                '#include <linux/futex.h>\n',
                args.cc,
                cte,
                exclude,
                linux_version_glibc > linux_version_headers,
                linux_version_headers > linux_version_glibc)

    status = max(
        check('FUTEX_WAITV_MAX'),
        check('FUTEX_PRIVATE_FLAG'),
        check('FUTEX_NO_NODE'),
        # glibc only defines the FUTEX2_* flags currently accepted by
        # the futex_waitv system call; the sizes other than 32-bit and
        # the size mask are not provided.
        check('FUTEX2_.*',
              'FUTEX2_SIZE_U8|FUTEX2_SIZE_U16|FUTEX2_SIZE_U64'
              '|FUTEX2_SIZE_MASK'))
    sys.exit(status)

if __name__ == '__main__':
    main()
