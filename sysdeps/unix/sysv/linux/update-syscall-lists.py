#!/usr/bin/python3
# Recreate <arch-syscall.h> and update syscall-names.list.
# Copyright (C) 2019 Free Software Foundation, Inc.
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

import argparse
import os
import sys

import glibcextract
import glibcsyscalls

def main():
    """The main entry point."""
    parser = argparse.ArgumentParser(
        description="System call list consistency checks")
    parser.add_argument('--cc', metavar='CC', required=True,
                        help='C compiler (including options) to use')
    parser.add_argument('arch_syscall', metavar='ARCH-SYSCALL-H',
                        help='The <arch-syscall.h> file to update')
    parser.add_argument('names_list', metavar='SYSCALL-NAMES-LIST',
                        help='The syscall name list to update ')

    args = parser.parse_args()

    kernel_constants = glibcsyscalls.kernel_constants(args.cc)

    # Replace <arch-syscall.h> with data derived from kernel headers.
    # No merging is necessary here.  Arch-specific changes should go
    # into <fixup-unistd-asm.h>.
    with open(args.arch_syscall, "r+") as out:
        os.lockf(out.fileno(), os.F_LOCK, 0)
        out.truncate()
        for name, value in sorted(kernel_constants.items()):
            out.write("#define __NR_{} {}\n".format(name, value))

    # Merge the architecture-specific system call names into the
    # global names list, syscall-names.list.  This file contains names
    # from other architectures (and comments), so it is necessary to
    # merge the existing files with the names obtained from the
    # kernel.
    with open(args.names_list, "r+") as list_file:
        os.lockf(list_file.fileno(), os.F_LOCK, 0)
        names_list = glibcsyscalls.SyscallNamesList(list_file)
        merged = names_list.merge(kernel_constants.keys())
        list_file.truncate()
        list_file.seek(0)
        for line in merged:
            list_file.write(line)

if __name__ == '__main__':
    main()
