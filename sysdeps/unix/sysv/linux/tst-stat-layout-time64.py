#!/usr/bin/python3
# Check that feature-test macros do not change the time64 stat layout.
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

MODES = {
    'POSIX.1-1996': '#define _POSIX_C_SOURCE 199506L',
    'POSIX.1-2001': '#define _POSIX_C_SOURCE 200112L',
    'POSIX.1-2008': '#define _POSIX_C_SOURCE 200809L',
    'XPG6': '#define _XOPEN_SOURCE 600',
}


def compute_stat_layout(cc, mode_define):
    # The nanosecond members are named differently in the two helper
    # branches: st_Xtim.tv_nsec with __USE_XOPEN2K8, st_Xtimensec
    #otherwise.
    sym_data = [
        '#undef _GNU_SOURCE',
        mode_define,
        '#define _TIME_BITS 64',
        '#define _FILE_OFFSET_BITS 64',
        '#include <stddef.h>',
        '#include <sys/stat.h>',
        'START',
        ('sizeof_stat', 'sizeof (struct stat)'),
        ('st_dev', 'offsetof (struct stat, st_dev)'),
        ('st_ino', 'offsetof (struct stat, st_ino)'),
        ('st_mode', 'offsetof (struct stat, st_mode)'),
        ('st_nlink', 'offsetof (struct stat, st_nlink)'),
        ('st_uid', 'offsetof (struct stat, st_uid)'),
        ('st_gid', 'offsetof (struct stat, st_gid)'),
        ('st_rdev', 'offsetof (struct stat, st_rdev)'),
        ('st_size', 'offsetof (struct stat, st_size)'),
        ('st_blksize', 'offsetof (struct stat, st_blksize)'),
        ('st_blocks', 'offsetof (struct stat, st_blocks)'),
        ('st_atime', 'offsetof (struct stat, st_atime)'),
        ('st_mtime', 'offsetof (struct stat, st_mtime)'),
        ('st_ctime', 'offsetof (struct stat, st_ctime)'),
        '#ifdef __USE_XOPEN2K8',
        ('st_atimensec', 'offsetof (struct stat, st_atim.tv_nsec)'),
        ('st_mtimensec', 'offsetof (struct stat, st_mtim.tv_nsec)'),
        ('st_ctimensec', 'offsetof (struct stat, st_ctim.tv_nsec)'),
        '#else',
        ('st_atimensec', 'offsetof (struct stat, st_atimensec)'),
        ('st_mtimensec', 'offsetof (struct stat, st_mtimensec)'),
        ('st_ctimensec', 'offsetof (struct stat, st_ctimensec)'),
        '#endif',
    ]
    return glibcextract.compute_c_consts(sym_data, cc)


def main():
    parser = argparse.ArgumentParser(
        description='Check that feature-test macros do not change '
        'the time64 stat layout.')
    parser.add_argument('--cc', metavar='CC',
                        help='C compiler (including options) to use')
    args = parser.parse_args()
    default_layout = compute_stat_layout(args.cc, '#define _GNU_SOURCE 1')
    status = 0
    for mode, mode_define in sorted(MODES.items()):
        mode_layout = compute_stat_layout(args.cc, mode_define)
        for name, value in default_layout.items():
            if mode_layout[name] != value:
                print('FAIL: %s: %s is %s, %s in default mode'
                      % (mode, name, mode_layout[name], value))
                status = 1
    if status == 0:
        print('PASS: struct stat layout is feature-test-macro invariant')
    sys.exit(status)


if __name__ == '__main__':
    main()
